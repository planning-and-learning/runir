#ifndef RUNIR_KR_DL_QUERY_DATA_HPP_
#define RUNIR_KR_DL_QUERY_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <boost/mp11.hpp>
#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <cstddef>
#include <optional>
#include <tuple>
#include <tyr/datalog/lifted/database/datalog/declarations.hpp>
#include <tyr/datalog/lifted/database/semantics/generic_join_plan.hpp>
#include <tyr/datalog/lifted/database/semantics/plans.hpp>
#include <tyr/formalism/declarations.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

// Query schemas, plans, and resolved positions are derived during checked
// construction. They are serialized with the node but do not define its identity.
template<>
struct data<runir::kr::dl::QueryColumn>
{
    urd::index<runir::kr::dl::QueryColumn> index;
    urd::flat_string name {};

    data() = default;
    explicit data(urd::flat_string name_) : index(), name(std::move(name_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name> {}; }
};

template<>
struct data<runir::kr::dl::Query<>>
{
private:
    template<typename Tag>
    using AlternativeIndex = urd::index<runir::kr::dl::Query<Tag>>;

public:
    using Variant = boost::mp11::mp_apply<cista::offset::variant, boost::mp11::mp_transform<AlternativeIndex, runir::kr::dl::QueryConstructorTags>>;

    urd::index<runir::kr::dl::Query<>> index;
    Variant variant;
    bool is_static = false;

    data() = default;
    // is_static is derived during insertion and therefore not a constructor parameter.
    explicit data(Variant variant_) : index(), variant(std::move(variant_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::variant, &data::is_static> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::variant> {}; }
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>> index;
    urd::index<tyr::formalism::Predicate<T>> predicate {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    data(urd::index<tyr::formalism::Predicate<T>> predicate_, urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_) :
        index(),
        predicate(std::move(predicate_)),
        columns(std::move(columns_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::predicate, &data::columns, &data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::predicate, &data::columns> {}; }
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>> index;
    urd::index<tyr::formalism::Predicate<T>> predicate {};
    bool polarity {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    data(urd::index<tyr::formalism::Predicate<T>> predicate_, bool polarity_, urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_) :
        index(),
        predicate(std::move(predicate_)),
        polarity(polarity_),
        columns(std::move(columns_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::predicate, &data::polarity, &data::columns, &data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::predicate, &data::polarity, &data::columns> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>> index;
    urd::index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>> arg {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    data(urd::index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>> arg_, urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_) :
        index(),
        arg(arg_),
        columns(std::move(columns_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg, &data::columns, &data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg, &data::columns> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryBooleanTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryBooleanTag>> index;
    urd::index<runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>> arg {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    data(urd::index<runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>> arg_, urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_) :
        index(),
        arg(arg_),
        columns(std::move(columns_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg, &data::columns, &data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg, &data::columns> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryNumericalTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryNumericalTag>> index;
    urd::index<runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>> arg {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    data(urd::index<runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>> arg_, urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_) :
        index(),
        arg(arg_),
        columns(std::move(columns_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg, &data::columns, &data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg, &data::columns> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>> index;
    urd::index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>> arg {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    data(urd::index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>> arg_, urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_) :
        index(),
        arg(arg_),
        columns(std::move(columns_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg, &data::columns, &data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg, &data::columns> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryJoinTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryJoinTag>> index;
    urd::index<runir::kr::dl::Query<>> lhs {};
    urd::index<runir::kr::dl::Query<>> rhs {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::JoinPlan plan {};

    data() = default;
    // columns and plan are derived during insertion and therefore not constructor parameters.
    data(urd::index<runir::kr::dl::Query<>> lhs_, urd::index<runir::kr::dl::Query<>> rhs_) : index(), lhs(lhs_), rhs(rhs_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::lhs, &data::rhs, &data::columns, &data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::lhs, &data::rhs> {}; }
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QueryUnionTag> || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
struct data<runir::kr::dl::Query<Tag>>
{
    urd::index<runir::kr::dl::Query<Tag>> index;
    urd::index<runir::kr::dl::Query<>> lhs {};
    urd::index<runir::kr::dl::Query<>> rhs {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::SetPlan plan {};

    data() = default;
    // columns and plan are derived during insertion and therefore not constructor parameters.
    data(urd::index<runir::kr::dl::Query<>> lhs_, urd::index<runir::kr::dl::Query<>> rhs_) : index(), lhs(lhs_), rhs(rhs_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::lhs, &data::rhs, &data::columns, &data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::lhs, &data::rhs> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryProjectTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryProjectTag>> index;
    urd::index<runir::kr::dl::Query<>> arg {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::ProjectionPlan plan {};

    data() = default;
    // plan is derived during insertion and therefore not a constructor parameter.
    data(urd::index<runir::kr::dl::Query<>> arg_, urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_) :
        index(),
        arg(arg_),
        columns(std::move(columns_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg, &data::columns, &data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg, &data::columns> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryRenameTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryRenameTag>> index;
    urd::index<runir::kr::dl::Query<>> arg {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    data(urd::index<runir::kr::dl::Query<>> arg_, urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_) :
        index(),
        arg(arg_),
        columns(std::move(columns_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg, &data::columns, &data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg, &data::columns> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>> index;
    urd::index<runir::kr::dl::Query<>> arg {};
    urd::index<runir::kr::dl::QueryColumn> lhs_column {};
    urd::index<runir::kr::dl::QueryColumn> rhs_column {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::SelectEqualPlan plan {};

    data() = default;
    // columns and plan are derived during insertion and therefore not constructor parameters.
    data(urd::index<runir::kr::dl::Query<>> arg_, urd::index<runir::kr::dl::QueryColumn> lhs_column_, urd::index<runir::kr::dl::QueryColumn> rhs_column_) :
        index(),
        arg(arg_),
        lhs_column(lhs_column_),
        rhs_column(rhs_column_)
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::arg, &data::lhs_column, &data::rhs_column, &data::columns, &data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg, &data::lhs_column, &data::rhs_column> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>> index;
    urd::index<runir::kr::dl::Query<>> arg {};
    urd::index<runir::kr::dl::QueryColumn> column {};
    urd::index<tyr::formalism::Object> object {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::SelectValuePlan plan {};

    data() = default;
    // columns and plan are derived during insertion and therefore not constructor parameters.
    data(urd::index<runir::kr::dl::Query<>> arg_, urd::index<runir::kr::dl::QueryColumn> column_, urd::index<tyr::formalism::Object> object_) :
        index(),
        arg(arg_),
        column(column_),
        object(object_)
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::arg, &data::column, &data::object, &data::columns, &data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg, &data::column, &data::object> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>> index;
    urd::flat_vector<urd::index<runir::kr::dl::Query<>>> args {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> variable_order {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> output_order {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::GenericJoinPlan plan {};

    data() = default;
    // columns and plan are derived during insertion and therefore not constructor parameters.
    data(urd::flat_vector<urd::index<runir::kr::dl::Query<>>> args_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> variable_order_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> output_order_) :
        index(),
        args(std::move(args_)),
        variable_order(std::move(variable_order_)),
        output_order(std::move(output_order_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::args, &data::variable_order, &data::output_order, &data::columns, &data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::args, &data::variable_order, &data::output_order> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>> index;
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    explicit data(urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_) : index(), columns(std::move(columns_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::columns, &data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::columns> {}; }
};

template<runir::kr::dl::ConceptOrRoleTag Category>
struct data<runir::kr::dl::QueryProjection<Category>>
{
    urd::index<runir::kr::dl::QueryProjection<Category>> index;
    urd::index<runir::kr::dl::Query<>> arg {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::ProjectionPlan plan {};

    data() = default;
    // plan is derived during insertion and therefore not a constructor parameter.
    data(urd::index<runir::kr::dl::Query<>> arg_, urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_) :
        index(),
        arg(arg_),
        columns(std::move(columns_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg, &data::columns, &data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg, &data::columns> {}; }
};

/// A Datalog program over the repository's DatalogRepository whose input slot s reads the rows of inputs[s].
template<>
struct data<runir::kr::dl::DatalogProgram>
{
    urd::index<runir::kr::dl::DatalogProgram> index;
    urd::index<tyr::datalog::database::datalog::Program> program {};
    urd::flat_vector<urd::index<runir::kr::dl::Query<>>> inputs {};
    /// The heads that features read (q_head); only these are evaluated, so the others may be evaluated partially.
    urd::flat_vector<urd::index<tyr::datalog::database::datalog::Relation>> outputs {};
    bool is_static = false;

    data() = default;
    // is_static is derived during insertion and therefore not a constructor parameter.
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::program, &data::inputs, &data::outputs, &data::is_static> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::program, &data::inputs, &data::outputs> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryHeadTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryHeadTag>> index;
    urd::index<runir::kr::dl::DatalogProgram> program {};
    urd::index<tyr::datalog::database::datalog::Relation> relation {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::program, &data::relation, &data::columns, &data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::program, &data::relation, &data::columns> {}; }
};

}  // namespace urd

#endif
