#ifndef RUNIR_KR_DL_QUERY_VIEW_HPP_
#define RUNIR_KR_DL_QUERY_VIEW_HPP_

#include "runir/kr/dl/query_data.hpp"

#include <string_view>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/database/datalog/repository.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::dl::QueryColumn>, C> : public ygg::IndexViewBase<runir::kr::dl::QueryColumn, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::QueryColumn, C>::IndexViewBase;

    const auto& get_name() const noexcept { return this->get_data().name; }
};

template<typename Tag, typename C>
class View<Index<runir::kr::dl::Query<Tag>>, C> : public ygg::IndexViewBase<runir::kr::dl::Query<Tag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::Query<Tag>, C>::IndexViewBase;

    auto get_variant() const noexcept
        requires std::same_as<Tag, void>
    { return make_view(this->get_data().variant, this->get_context()); }

    [[nodiscard]] bool is_static() const noexcept
        requires std::same_as<Tag, void>
    { return this->get_data().is_static; }

    auto get_arg() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryConceptTag> || std::same_as<Tag, runir::kr::dl::QueryRoleTag>
                 || std::same_as<Tag, runir::kr::dl::QueryProjectTag> || std::same_as<Tag, runir::kr::dl::QueryRenameTag>
                 || std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag> || std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>)
    { return make_view(this->get_data().arg, this->get_context()); }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    { return make_view(this->get_data().lhs, this->get_context()); }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    { return make_view(this->get_data().rhs, this->get_context()); }

    auto get_args() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    { return make_view(this->get_data().args, this->get_context()); }

    auto get_variable_order() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    { return make_view(this->get_data().variable_order, this->get_context()); }

    auto get_output_order() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>
    { return make_view(this->get_data().output_order, this->get_context()); }

    auto get_keys() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    { return make_view(this->get_data().keys, this->get_context()); }

    auto get_sources() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    { return make_view(this->get_data().sources, this->get_context()); }

    auto get_edges() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    { return make_view(this->get_data().edges, this->get_context()); }

    auto get_targets() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    { return make_view(this->get_data().targets, this->get_context()); }

    auto get_probes() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryMustPassTag>
    { return make_view(this->get_data().probes, this->get_context()); }

    auto get_program() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryHeadTag>
    { return make_view(this->get_data().program, this->get_context()); }

    /// The head relation in the repository's DatalogRepository.
    auto get_relation() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryHeadTag>
    { return ygg::database::datalog::RelationView(this->get_data().relation, this->get_context().get_datalog_repository()); }

    /// The head relation's name in its program.
    [[nodiscard]] std::string_view get_name() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QueryHeadTag>
    {
        const auto& names = get_program().get_data().relation_names;
        const auto id = get_relation().get_id();
        return id < names.size() ? std::string_view(names[id]) : std::string_view();
    }

    auto get_columns() const noexcept
    {
        if constexpr (std::same_as<Tag, void>)
            return ygg::visit([](auto concrete) { return concrete.get_columns(); }, get_variant());
        else
            return make_view(this->get_data().columns, this->get_context());
    }

    auto get_schema() const noexcept
    {
        if constexpr (std::same_as<Tag, void>)
            return ygg::visit([](auto concrete) { return concrete.get_schema(); }, get_variant());
        else if constexpr (std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryProjectTag>)
            return make_view(this->get_data().plan.output_columns().get_record(), this->get_context());
        else if constexpr (std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>)
            return make_view(this->get_data().plan.output_columns().get_data(), this->get_context());
        else
            return make_view(this->get_data().schema.get_data(), this->get_context());
    }

    auto get_lhs_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    { return make_view(this->get_data().lhs_column, this->get_context()); }

    auto get_rhs_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    { return make_view(this->get_data().rhs_column, this->get_context()); }

    auto get_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    { return make_view(this->get_data().column, this->get_context()); }

    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    { return make_view(this->get_data().predicate, this->get_context().get_planning_repository()); }

    auto get_object() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    { return make_view(this->get_data().object, this->get_context().get_planning_repository()); }

    auto get_polarity() const noexcept
        requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
    { return this->get_data().polarity; }
};

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
class View<Index<runir::kr::dl::QueryProjection<Category>>, C> : public ygg::IndexViewBase<runir::kr::dl::QueryProjection<Category>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::QueryProjection<Category>, C>::IndexViewBase;

    auto get_arg() const noexcept { return make_view(this->get_data().arg, this->get_context()); }
    auto get_columns() const noexcept { return make_view(this->get_data().columns, this->get_context()); }
};

template<typename C>
class View<Index<runir::kr::dl::DatalogProgram>, C> : public ygg::IndexViewBase<runir::kr::dl::DatalogProgram, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::DatalogProgram, C>::IndexViewBase;

    /// The program's rules, in the repository's DatalogRepository.
    auto get_program() const noexcept { return ygg::database::datalog::ProgramView(this->get_data().program, this->get_context().get_datalog_repository()); }
    auto get_inputs() const noexcept { return make_view(this->get_data().inputs, this->get_context()); }
    const auto& get_relation_names() const noexcept { return this->get_data().relation_names; }
    const auto& get_input_names() const noexcept { return this->get_data().input_names; }
    [[nodiscard]] bool is_static() const noexcept { return this->get_data().is_static; }
};

}  // namespace ygg

#endif
