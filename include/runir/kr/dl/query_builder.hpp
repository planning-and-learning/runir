#ifndef RUNIR_KR_DL_QUERY_BUILDER_HPP_
#define RUNIR_KR_DL_QUERY_BUILDER_HPP_

#include "runir/kr/dl/query_data.hpp"
#include "runir/kr/dl/query_view.hpp"

#include <optional>
#include <tyr/formalism/object_builder.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/formalism/predicate_builder.hpp>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of QueryColumn: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::QueryColumn, C>
{
    urd::index<runir::kr::dl::QueryColumn> index;
    urd::built_t<urd::flat_string, C> name {};

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"name", &builder::name>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::name> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<void>, C>
{
    urd::index<runir::kr::dl::Query<>> index;
    urd::built_t<runir::kr::dl::QueryVariant, C> variant;
    urd::built_t<bool, C> is_static = false;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"variant", &builder::variant>, urd::field<"is_static", &builder::is_static>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::variant> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename T, typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>> index;
    urd::built_t<urd::index<tyr::formalism::Predicate<T>>, ::tyr::formalism::planning::Repository> predicate {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> schema {};

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"predicate", &builder::predicate>, urd::field<"columns", &builder::columns>, urd::field<"schema", &builder::schema>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::predicate, &builder::columns> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename T, typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>> index;
    urd::built_t<urd::index<tyr::formalism::Predicate<T>>, ::tyr::formalism::planning::Repository> predicate {};
    urd::built_t<bool, C> polarity {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> schema {};

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"predicate", &builder::predicate>,
                           urd::field<"polarity", &builder::polarity>,
                           urd::field<"columns", &builder::columns>,
                           urd::field<"schema", &builder::schema>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::predicate, &builder::polarity, &builder::columns> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>, C> operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> schema {};

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"columns", &builder::columns>, urd::field<"schema", &builder::schema>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::columns> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>, C> operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> schema {};

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"columns", &builder::columns>, urd::field<"schema", &builder::schema>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::columns> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryJoinTag>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryJoinTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> left_operand {};
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> right_operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<tyr::datalog::database::JoinPlan, C> plan {};

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &builder::left_operand>,
                           urd::field<"right_operand", &builder::right_operand>,
                           urd::field<"columns", &builder::columns>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::left_operand, &builder::right_operand> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename Tag, typename C>
    requires boost::mp11::mp_contains<runir::kr::dl::QuerySetTags, Tag>::value
struct builder<runir::kr::dl::Query<Tag>, C>
{
    urd::index<runir::kr::dl::Query<Tag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> left_operand {};
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> right_operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<tyr::datalog::database::SetPlan, C> plan {};

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &builder::left_operand>,
                           urd::field<"right_operand", &builder::right_operand>,
                           urd::field<"columns", &builder::columns>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::left_operand, &builder::right_operand> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryProjectTag>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryProjectTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<tyr::datalog::database::ProjectionPlan, C> plan {};

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"columns", &builder::columns>, urd::field<"plan", &builder::plan>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::columns> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryRenameTag>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryRenameTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> schema {};

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"columns", &builder::columns>, urd::field<"schema", &builder::schema>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::columns> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> operand {};
    urd::built_t<urd::index<runir::kr::dl::QueryColumn>, C> left_column {};
    urd::built_t<urd::index<runir::kr::dl::QueryColumn>, C> right_column {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<tyr::datalog::database::SelectEqualPlan, C> plan {};

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"operand", &builder::operand>,
                           urd::field<"left_column", &builder::left_column>,
                           urd::field<"right_column", &builder::right_column>,
                           urd::field<"columns", &builder::columns>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::left_column, &builder::right_column> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> operand {};
    urd::built_t<urd::index<runir::kr::dl::QueryColumn>, C> column {};
    urd::built_t<urd::index<tyr::formalism::Object>, ::tyr::formalism::planning::Repository> object {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<tyr::datalog::database::SelectValuePlan, C> plan {};

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"operand", &builder::operand>,
                           urd::field<"column", &builder::column>,
                           urd::field<"object", &builder::object>,
                           urd::field<"columns", &builder::columns>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::column, &builder::object> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Query<>>>, C> args {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> variable_order {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> output_order {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<tyr::datalog::database::GenericJoinPlan, C> plan {};

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"args", &builder::args>,
                           urd::field<"variable_order", &builder::variable_order>,
                           urd::field<"output_order", &builder::output_order>,
                           urd::field<"columns", &builder::columns>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::args, &builder::variable_order, &builder::output_order> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryMustPassTag>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryMustPassTag>> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> keys {};
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> sources {};
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> edges {};
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> targets {};
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> probes {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<tyr::datalog::database::MustPassPlan, C> plan {};

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"keys", &builder::keys>,
                           urd::field<"sources", &builder::sources>,
                           urd::field<"edges", &builder::edges>,
                           urd::field<"targets", &builder::targets>,
                           urd::field<"probes", &builder::probes>,
                           urd::field<"columns", &builder::columns>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept
    { return urd::Members<&builder::keys, &builder::sources, &builder::edges, &builder::targets, &builder::probes> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>, C>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> schema {};

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"columns", &builder::columns>, urd::field<"schema", &builder::schema>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::columns> {}; }
};

/// The builder of QueryProjection: its record's members, each in its built form (urd::built_t).
template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
struct builder<runir::kr::dl::QueryProjection<Category>, C>
{
    urd::index<runir::kr::dl::QueryProjection<Category>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<tyr::datalog::database::ProjectionPlan, C> plan {};

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"columns", &builder::columns>, urd::field<"plan", &builder::plan>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::columns> {}; }
};

}  // namespace urd

#endif
