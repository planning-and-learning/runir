#ifndef RUNIR_KR_DL_QUERY_VIEW_HPP_
#define RUNIR_KR_DL_QUERY_VIEW_HPP_

#include "runir/kr/dl/query_data.hpp"

#include <cstddef>
#include <ranges>
#include <runir/kr/database/datalog/repository.hpp>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/elements/flat.hpp>

namespace runir::kr::dl::semantics
{
/// The syntactic complexity of a query and of a query projection (defined in syntactic_complexity.hpp), declared
/// before their views, whose functions() list it.
template<typename Tag, typename C>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::Query<Tag>, C>& view);

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::QueryProjection<Category>, C>& view);

std::size_t syntactic_complexity(const runir::kr::dl::QueryView<>& view);
}  // namespace runir::kr::dl::semantics

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
    { return self().template member<&urd::record_of<V>::variant>(); }

    [[nodiscard]] bool get_is_static() const noexcept
        requires std::same_as<Tag, void>
    { return self().template member<&urd::record_of<V>::is_static>(); }

    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    { return self().template member<&urd::record_of<V>::predicate>(); }

    auto get_polarity() const noexcept
        requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
    { return self().template member<&urd::record_of<V>::polarity>(); }

    auto get_operand() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryConceptTag> || std::same_as<Tag, runir::kr::dl::QueryRoleTag>
                 || std::same_as<Tag, runir::kr::dl::QueryBooleanTag> || std::same_as<Tag, runir::kr::dl::QueryNumericalTag>
                 || std::same_as<Tag, runir::kr::dl::QueryProjectTag> || std::same_as<Tag, runir::kr::dl::QueryRenameTag>
                 || std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag> || std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>)
    { return self().template member<&urd::record_of<V>::operand>(); }

    auto get_left_operand() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    { return self().template member<&urd::record_of<V>::left_operand>(); }

    auto get_right_operand() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    { return self().template member<&urd::record_of<V>::right_operand>(); }

    auto get_left_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    { return self().template member<&urd::record_of<V>::left_column>(); }

    auto get_right_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    { return self().template member<&urd::record_of<V>::right_column>(); }

    auto get_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    { return self().template member<&urd::record_of<V>::column>(); }

    auto get_object() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    { return self().template member<&urd::record_of<V>::object>(); }

    auto get_args() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    { return self().template member<&urd::record_of<V>::args>(); }

    auto get_variable_order() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    { return self().template member<&urd::record_of<V>::variable_order>(); }

    auto get_output_order() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    { return self().template member<&urd::record_of<V>::output_order>(); }

    auto get_program() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryHeadTag>
    { return self().template member<&urd::record_of<V>::program>(); }

    /// The head relation in the repository's DatalogRepository.
    auto get_relation() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryHeadTag>
    {
        return self().context().get_datalog_repository().view(
            urd::index<runir::kr::database::datalog::Relation>(self().template member<&urd::record_of<V>::relation>()));
    }

    auto get_columns() const noexcept
        requires(!std::same_as<Tag, void>)
    { return self().template member<&urd::record_of<V>::columns>(); }

    /// Its plan, derived when it was interned.
    const auto& get_plan() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag> || std::same_as<Tag, runir::kr::dl::QueryProjectTag>
                 || std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag> || std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
                 || std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>)
    { return self().template member<&urd::record_of<V>::plan>(); }

    [[nodiscard]] std::span<const urd::data<ygg::ColumnLayout>> get_schema() const noexcept
        requires(!std::same_as<Tag, void>)
    {
        if constexpr (requires { self().template member<&urd::record_of<V>::plan>().output_columns(); })
            return self().template member<&urd::record_of<V>::plan>().output_columns();
        else
            return { self().template member<&urd::record_of<V>::schema>().data(), self().template member<&urd::record_of<V>::schema>().size() };
    }

    static constexpr auto functions() noexcept
    {
        return urd::Functions<
            urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
    }
};

template<typename V>
struct view_mixin<runir::kr::dl::DatalogProgram, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    /// The program's rules, in the repository's DatalogRepository.
    auto get_program() const noexcept
    {
        return self().context().get_datalog_repository().view(
            urd::index<runir::kr::database::datalog::Program>(self().template member<&urd::record_of<V>::program>()));
    }

    auto get_inputs() const noexcept { return self().template member<&urd::record_of<V>::inputs>(); }

    /// The heads that features read, in the repository's DatalogRepository.
    auto get_outputs() const noexcept
    {
        return self().template member<&urd::record_of<V>::outputs>()
               | std::views::transform([repository = &self().context().get_datalog_repository()](ygg::uint_t relation)
                                       { return repository->view(urd::index<runir::kr::database::datalog::Relation>(relation)); });
    }

    [[nodiscard]] bool get_is_static() const noexcept { return self().template member<&urd::record_of<V>::is_static>(); }
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

    static constexpr auto functions() noexcept
    {
        return urd::Functions<
            urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
    }
};
}  // namespace urd

namespace runir::kr::dl
{
/// The columns of a query, read through its variant for a query of any operation (its operation's).
template<typename Tag>
auto columns_of(QueryView<Tag> view)
{
    if constexpr (std::same_as<Tag, void>)
        return std::visit([](auto concrete) { return concrete.get_columns(); }, view.get_variant());
    else
        return view.get_columns();
}

/// The schema of a query, read through its variant for a query of any operation (its operation's).
template<typename Tag>
std::span<const urd::data<ygg::ColumnLayout>> schema_of(QueryView<Tag> view)
{
    if constexpr (std::same_as<Tag, void>)
        return std::visit([](auto concrete) { return concrete.get_schema(); }, view.get_variant());
    else
        return view.get_schema();
}
}  // namespace runir::kr::dl

#endif
