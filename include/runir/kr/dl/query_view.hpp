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
    const auto& get_name() const noexcept { return self().template member<&urd::record_of<V>::name>(); }
};

template<typename Tag, typename V>
struct view_mixin<runir::kr::dl::Query<Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_variant() const noexcept
        requires std::same_as<Tag, void>
    {
        return self().template member<&urd::record_of<V>::variant>();
    }

    [[nodiscard]] bool is_static() const noexcept
        requires std::same_as<Tag, void>
    {
        return self().template member<&urd::record_of<V>::is_static>();
    }

    auto get_operand() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryConceptTag> || std::same_as<Tag, runir::kr::dl::QueryRoleTag>
                 || std::same_as<Tag, runir::kr::dl::QueryProjectTag> || std::same_as<Tag, runir::kr::dl::QueryRenameTag>
                 || std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag> || std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>)
    {
        return self().template member<&urd::record_of<V>::operand>();
    }

    auto get_left_operand() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    {
        return self().template member<&urd::record_of<V>::left_operand>();
    }

    auto get_right_operand() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    {
        return self().template member<&urd::record_of<V>::right_operand>();
    }

    auto get_args() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    {
        return self().template member<&urd::record_of<V>::args>();
    }

    auto get_variable_order() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    {
        return self().template member<&urd::record_of<V>::variable_order>();
    }

    auto get_output_order() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    {
        return self().template member<&urd::record_of<V>::output_order>();
    }

    auto get_keys() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    {
        return self().template member<&urd::record_of<V>::keys>();
    }

    auto get_sources() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    {
        return self().template member<&urd::record_of<V>::sources>();
    }

    auto get_edges() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    {
        return self().template member<&urd::record_of<V>::edges>();
    }

    auto get_targets() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    {
        return self().template member<&urd::record_of<V>::targets>();
    }

    auto get_probes() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    {
        return self().template member<&urd::record_of<V>::probes>();
    }

    auto get_columns() const noexcept
    {
        if constexpr (std::same_as<Tag, void>)
            return std::visit([](auto concrete) { return concrete.get_columns(); }, get_variant());
        else
            return self().template member<&urd::record_of<V>::columns>();
    }

    /// Its plan, derived when it was interned.
    const auto& get_plan() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag> || std::same_as<Tag, runir::kr::dl::QueryProjectTag>
                 || std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag> || std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
                 || std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag> || std::same_as<Tag, runir::kr::dl::QueryMustPassTag>)
    {
        return self().template member<&urd::record_of<V>::plan>();
    }

    [[nodiscard]] std::span<const ygg::ColumnLayout> get_schema() const noexcept
    {
        if constexpr (std::same_as<Tag, void>)
            return std::visit([](auto concrete) { return concrete.get_schema(); }, get_variant());
        else if constexpr (requires { self().template member<&urd::record_of<V>::plan>().output_columns(); })
            return self().template member<&urd::record_of<V>::plan>().output_columns();
        else
            return { self().template member<&urd::record_of<V>::schema>().data(), self().template member<&urd::record_of<V>::schema>().size() };
    }

    auto get_left_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    {
        return self().template member<&urd::record_of<V>::left_column>();
    }

    auto get_right_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    {
        return self().template member<&urd::record_of<V>::right_column>();
    }

    auto get_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    {
        return self().template member<&urd::record_of<V>::column>();
    }

    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    {
        return self().template member<&urd::record_of<V>::predicate>();
    }

    auto get_object() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    {
        return self().template member<&urd::record_of<V>::object>();
    }

    auto get_polarity() const noexcept
        requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
    {
        return self().template member<&urd::record_of<V>::polarity>();
    }
};

template<runir::kr::dl::ConceptOrRoleTag Category, typename V>
struct view_mixin<runir::kr::dl::QueryProjection<Category>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }
    auto get_columns() const noexcept { return self().template member<&urd::record_of<V>::columns>(); }
    /// Its plan, derived when it was interned.
    const auto& get_plan() const noexcept { return self().template member<&urd::record_of<V>::plan>(); }
};
}  // namespace urd

#endif
