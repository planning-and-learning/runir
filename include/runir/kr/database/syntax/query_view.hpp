/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_SYNTAX_QUERY_VIEW_HPP_
#define RUNIR_KR_DATABASE_SYNTAX_QUERY_VIEW_HPP_

#include "runir/kr/database/syntax/query_data.hpp"

#include <boost/mp11.hpp>
#include <concepts>
#include <span>
#include <tuple>
#include <urd/elements/children.hpp>
#include <urd/elements/flat.hpp>

namespace urd
{
template<typename Tag, typename V>
struct view_mixin<runir::kr::database::Query<Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] auto variant() const
        requires std::same_as<Tag, void>
    { return self().template member<&urd::record_of<V>::variant>(); }

    [[nodiscard]] auto operand() const
        requires boost::mp11::mp_contains<runir::kr::database::QueryUnaryTags, Tag>::value
    { return self().template member<&urd::record_of<V>::operand>(); }

    [[nodiscard]] auto left_operand() const
        requires boost::mp11::mp_contains<runir::kr::database::QueryBinaryTags, Tag>::value
    { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] auto right_operand() const
        requires boost::mp11::mp_contains<runir::kr::database::QueryBinaryTags, Tag>::value
    { return self().template member<&urd::record_of<V>::right_operand>(); }

    [[nodiscard]] auto input_slot() const
        requires std::same_as<Tag, runir::kr::database::QueryInputTag>
    { return self().template member<&urd::record_of<V>::input_slot>(); }

    [[nodiscard]] auto labels() const
        requires(std::same_as<Tag, runir::kr::database::QueryProjectTag> || std::same_as<Tag, runir::kr::database::QueryRenameTag>)
    {
        return std::span<const urd::index<ygg::Column>>(self().template member<&urd::record_of<V>::labels>().data(),
                                                        self().template member<&urd::record_of<V>::labels>().size());
    }

    [[nodiscard]] auto left_column() const
        requires std::same_as<Tag, runir::kr::database::QuerySelectEqualTag>
    { return self().template member<&urd::record_of<V>::left_column>(); }

    [[nodiscard]] auto right_column() const
        requires std::same_as<Tag, runir::kr::database::QuerySelectEqualTag>
    { return self().template member<&urd::record_of<V>::right_column>(); }

    [[nodiscard]] auto column() const
        requires std::same_as<Tag, runir::kr::database::QuerySelectValueTag>
    { return self().template member<&urd::record_of<V>::column>(); }

    [[nodiscard]] auto constant() const
        requires std::same_as<Tag, runir::kr::database::QuerySelectValueTag>
    { return self().template member<&urd::record_of<V>::constant>(); }

    [[nodiscard]] auto constant_type() const
        requires std::same_as<Tag, runir::kr::database::QuerySelectValueTag>
    { return self().template member<&urd::record_of<V>::constant_type>(); }

    [[nodiscard]] auto inputs() const
        requires std::same_as<Tag, runir::kr::database::QueryGenericJoinTag>
    { return self().template member<&urd::record_of<V>::inputs>(); }

    [[nodiscard]] auto variable_order() const
        requires std::same_as<Tag, runir::kr::database::QueryGenericJoinTag>
    {
        return std::span<const urd::index<ygg::Column>>(self().template member<&urd::record_of<V>::variable_order>().data(),
                                                        self().template member<&urd::record_of<V>::variable_order>().size());
    }

    [[nodiscard]] auto output_order() const
        requires std::same_as<Tag, runir::kr::database::QueryGenericJoinTag>
    {
        return std::span<const urd::index<ygg::Column>>(self().template member<&urd::record_of<V>::output_order>().data(),
                                                        self().template member<&urd::record_of<V>::output_order>().size());
    }

    [[nodiscard]] const auto& plan() const
        requires boost::mp11::mp_contains<runir::kr::database::QueryPlannedTags, Tag>::value
    { return self().template member<&urd::record_of<V>::plan>(); }

    [[nodiscard]] std::span<const urd::data<ygg::ColumnLayout>> columns() const
        requires(!std::same_as<Tag, void>)
    {
        if constexpr (std::same_as<Tag, runir::kr::database::QuerySemijoinTag> || std::same_as<Tag, runir::kr::database::QueryAntijoinTag>)
            return plan().left_columns;
        else if constexpr (boost::mp11::mp_contains<runir::kr::database::QueryPlannedTags, Tag>::value)
            return plan().output_columns;
        else
        {
            // An input, an empty query and a rename store their columns.
            const auto& stored = self().template member<&urd::record_of<V>::columns>();
            return { stored.data(), stored.size() };
        }
    }

    static constexpr auto functions() noexcept
    {
        // A query that stores its columns reads them by its field's getter.
        if constexpr (std::same_as<Tag, void> || std::same_as<Tag, runir::kr::database::QueryInputTag> || std::same_as<Tag, runir::kr::database::QueryEmptyTag>
                      || std::same_as<Tag, runir::kr::database::QueryRenameTag>)
            return urd::Functions<> {};
        else
            return urd::Functions<urd::function<"columns", &view_mixin::columns>> {};
    }
};
}  // namespace urd

namespace runir::kr::database
{
/// Calls f(QueryView<>) for every operand of a query, in operand order.
template<typename Tag, std::invocable<QueryView<>> F>
void for_each_child(QueryView<Tag> view, F f)
{
    if constexpr (std::same_as<Tag, void>)
        std::visit([&](auto concrete) { for_each_child(concrete, f); }, view.variant());
    else
        urd::for_each_child<boost::mp11::mp_list<Query<>>, Query<Tag>>(view, f);
}

/// The columns of a query, read through its variant for a query of any operation (its operation's).
template<typename Tag>
[[nodiscard]] std::span<const urd::data<ygg::ColumnLayout>> columns_of(QueryView<Tag> view)
{
    if constexpr (std::same_as<Tag, void>)
        return std::visit([](auto concrete) { return concrete.columns(); }, view.variant());
    else
        return view.columns();
}
}  // namespace runir::kr::database

#endif
