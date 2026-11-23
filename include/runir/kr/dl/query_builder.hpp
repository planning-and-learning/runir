#ifndef RUNIR_KR_DL_QUERY_BUILDER_HPP_
#define RUNIR_KR_DL_QUERY_BUILDER_HPP_

#include "runir/kr/dl/query_data.hpp"
#include "runir/kr/dl/query_view.hpp"

#include <optional>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/object_builder.hpp>
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
    urd::key<runir::kr::dl::QueryColumn> index;
    urd::built_t<urd::flat_string, C> name {};

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"name", &builder::name>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::name> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<void>, C>
{
    urd::key<runir::kr::dl::Query<>> index;
    urd::built_t<runir::kr::dl::QueryVariant, C> variant;
    urd::built_t<bool, C> is_static = false;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"variant", &builder::variant>, urd::field<"is_static", &builder::is_static>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::variant> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename T, typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>, C>
{
    urd::key<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>> index;
    urd::built_t<urd::index<tyr::formalism::Predicate<T>>, ::tyr::formalism::Repository> predicate {};
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
    urd::key<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>> index;
    urd::built_t<urd::index<tyr::formalism::Predicate<T>>, ::tyr::formalism::Repository> predicate {};
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
    urd::key<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>> index;
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
    urd::key<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>> index;
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
    urd::key<runir::kr::dl::Query<runir::kr::dl::QueryJoinTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> left_operand {};
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> right_operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::data<runir::kr::database::JoinPlan>, C> plan {};

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
    urd::key<runir::kr::dl::Query<Tag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> left_operand {};
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> right_operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::data<runir::kr::database::SetPlan>, C> plan {};

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
    urd::key<runir::kr::dl::Query<runir::kr::dl::QueryProjectTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::data<runir::kr::database::ProjectionPlan>, C> plan {};

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"columns", &builder::columns>, urd::field<"plan", &builder::plan>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::columns> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryRenameTag>, C>
{
    urd::key<runir::kr::dl::Query<runir::kr::dl::QueryRenameTag>> index;
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
    urd::key<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> operand {};
    urd::built_t<urd::index<runir::kr::dl::QueryColumn>, C> left_column {};
    urd::built_t<urd::index<runir::kr::dl::QueryColumn>, C> right_column {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::data<runir::kr::database::SelectEqualPlan>, C> plan {};

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
    urd::key<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> operand {};
    urd::built_t<urd::index<runir::kr::dl::QueryColumn>, C> column {};
    urd::built_t<urd::index<tyr::formalism::Object>, ::tyr::formalism::Repository> object {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::data<runir::kr::database::SelectValuePlan>, C> plan {};

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
    urd::key<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Query<>>>, C> args {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> variable_order {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> output_order {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::data<runir::kr::database::GenericJoinPlan>, C> plan {};

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
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>, C>
{
    urd::key<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> schema {};

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"columns", &builder::columns>, urd::field<"schema", &builder::schema>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::columns> {}; }
};

/// The builder of QueryProjection: its record's members, each in its built form (urd::built_t).
template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
struct builder<runir::kr::dl::QueryProjection<Category>, C>
{
    urd::key<runir::kr::dl::QueryProjection<Category>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::data<runir::kr::database::ProjectionPlan>, C> plan {};

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"columns", &builder::columns>, urd::field<"plan", &builder::plan>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::columns> {}; }
};

/// The builder of QueryBooleanTag>: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryBooleanTag>, C>
{
    urd::key<runir::kr::dl::Query<runir::kr::dl::QueryBooleanTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>, C> operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> schema {};

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"columns", &builder::columns>, urd::field<"schema", &builder::schema>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::columns> {}; }
};

/// The builder of QueryNumericalTag>: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryNumericalTag>, C>
{
    urd::key<runir::kr::dl::Query<runir::kr::dl::QueryNumericalTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>>, C> operand {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> schema {};

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"columns", &builder::columns>, urd::field<"schema", &builder::schema>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::columns> {}; }
};

/// The builder of DatalogProgram: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::DatalogProgram, C>
{
    urd::key<runir::kr::dl::DatalogProgram> index;
    urd::built_t<ygg::uint_t, C> program {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Query<>>>, C> inputs {};
    urd::built_t<urd::flat_vector<ygg::uint_t>, C> outputs {};
    urd::built_t<bool, C> is_static {};

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"program", &builder::program>,
                           urd::field<"inputs", &builder::inputs>,
                           urd::field<"outputs", &builder::outputs>,
                           urd::field<"is_static", &builder::is_static>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::program, &builder::inputs, &builder::outputs> {}; }
};

/// The builder of QueryHeadTag>: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Query<runir::kr::dl::QueryHeadTag>, C>
{
    urd::key<runir::kr::dl::Query<runir::kr::dl::QueryHeadTag>> index;
    urd::built_t<urd::index<runir::kr::dl::DatalogProgram>, C> program {};
    urd::built_t<ygg::uint_t, C> relation {};
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>>, C> columns {};
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> schema {};

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"program", &builder::program>,
                           urd::field<"relation", &builder::relation>,
                           urd::field<"columns", &builder::columns>,
                           urd::field<"schema", &builder::schema>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::program, &builder::relation, &builder::columns> {}; }
};

}  // namespace urd

#endif
