#ifndef RUNIR_KR_DL_QUERY_VIEW_HPP_
#define RUNIR_KR_DL_QUERY_VIEW_HPP_

#include "runir/kr/dl/query_data.hpp"

#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/elements/flat.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::dl::QueryColumn, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_name() const noexcept { return self().data().name; }
};

template<typename Tag, typename V>
struct view_mixin<runir::kr::dl::Query<Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_variant() const noexcept
        requires std::same_as<Tag, void>
    { return self().template get<&urd::record_of<V>::variant>(); }

    [[nodiscard]] bool is_static() const noexcept
        requires std::same_as<Tag, void>
    { return self().data().is_static; }

    auto get_arg() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryConceptTag> || std::same_as<Tag, runir::kr::dl::QueryRoleTag>
                 || std::same_as<Tag, runir::kr::dl::QueryProjectTag> || std::same_as<Tag, runir::kr::dl::QueryRenameTag>
                 || std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag> || std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>)
    { return self().template get<&urd::record_of<V>::arg>(); }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    { return self().template get<&urd::record_of<V>::lhs>(); }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    { return self().template get<&urd::record_of<V>::rhs>(); }

    auto get_args() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    { return self().template get<&urd::record_of<V>::args>(); }

    auto get_variable_order() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    { return self().template get<&urd::record_of<V>::variable_order>(); }

    auto get_output_order() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    { return self().template get<&urd::record_of<V>::output_order>(); }

    auto get_keys() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    { return self().template get<&urd::record_of<V>::keys>(); }

    auto get_sources() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    { return self().template get<&urd::record_of<V>::sources>(); }

    auto get_edges() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    { return self().template get<&urd::record_of<V>::edges>(); }

    auto get_targets() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    { return self().template get<&urd::record_of<V>::targets>(); }

    auto get_probes() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    { return self().template get<&urd::record_of<V>::probes>(); }

    auto get_columns() const noexcept
    {
        if constexpr (std::same_as<Tag, void>)
            return std::visit([](auto concrete) { return concrete.get_columns(); }, get_variant());
        else
            return self().template get<&urd::record_of<V>::columns>();
    }

    std::span<const ygg::ColumnLayout> get_schema() const noexcept
    {
        if constexpr (std::same_as<Tag, void>)
            return std::visit([](auto concrete) { return concrete.get_schema(); }, get_variant());
        else if constexpr (requires { self().data().plan.output_columns(); })
            return self().data().plan.output_columns();
        else
            return { self().data().schema.data(), self().data().schema.size() };
    }

    auto get_lhs_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    { return self().template get<&urd::record_of<V>::lhs_column>(); }

    auto get_rhs_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    { return self().template get<&urd::record_of<V>::rhs_column>(); }

    auto get_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    { return self().template get<&urd::record_of<V>::column>(); }

    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    { return urd::resolve(self().data().predicate, self().context().get_planning_repository()); }

    auto get_object() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    { return urd::resolve(self().data().object, self().context().get_planning_repository()); }

    auto get_polarity() const noexcept
        requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
    { return self().data().polarity; }
};

template<runir::kr::dl::ConceptOrRoleTag Category, typename V>
struct view_mixin<runir::kr::dl::QueryProjection<Category>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_arg() const noexcept { return self().template get<&urd::record_of<V>::arg>(); }
    auto get_columns() const noexcept { return self().template get<&urd::record_of<V>::columns>(); }
};
}  // namespace urd

#endif
