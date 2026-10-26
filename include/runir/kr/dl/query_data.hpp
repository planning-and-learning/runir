#ifndef RUNIR_KR_DL_QUERY_DATA_HPP_
#define RUNIR_KR_DL_QUERY_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <cista/containers/vector.h>
#include <cstddef>
#include <optional>
#include <tuple>
#include <tyr/formalism/declarations.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/database/datalog/declarations.hpp>
#include <yggdrasil/database/operators/generic_join/plan.hpp>
#include <yggdrasil/database/operators/relational/plan.hpp>
#include <yggdrasil/database/search/must_pass_plan.hpp>

namespace ygg
{

// Query schemas, plans, and resolved positions are derived during checked
// construction. They are serialized with the node but do not define its identity.
template<>
struct Data<runir::kr::dl::QueryColumn>
{
    Index<runir::kr::dl::QueryColumn> index;
    cista::offset::string name {};

    Data() = default;
    explicit Data(cista::offset::string name_) : index(), name(std::move(name_)) {}

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::name> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::name> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<>>
{
private:
    template<typename Tag>
    using AlternativeIndex = ygg::Index<runir::kr::dl::Query<Tag>>;

public:
    using Variant = ygg::ApplyTypeListT<cista::offset::variant, ygg::MapTypeListT<AlternativeIndex, runir::kr::dl::QueryConstructorTags>>;

    Index<runir::kr::dl::Query<>> index;
    Variant variant;
    bool is_static = false;

    Data() = default;
    // is_static is derived during insertion and therefore not a constructor parameter.
    explicit Data(Variant variant_) : index(), variant(std::move(variant_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<Variant, C>;
    template<typename C>
    explicit Data(const ViewVariant<C>& variant_) :
        index(),
        variant(std::visit([](const auto& view) -> Variant { return Variant(view.get_index()); }, variant_))
    {
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::variant, &Data::is_static> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::variant> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>> index;
    Index<tyr::formalism::Predicate<T>> predicate {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    Data(Index<tyr::formalism::Predicate<T>> predicate_, IndexList<runir::kr::dl::QueryColumn> columns_) :
        index(),
        predicate(std::move(predicate_)),
        columns(std::move(columns_))
    {
    }
    template<typename C, typename P>
    Data(::ygg::View<Index<tyr::formalism::Predicate<T>>, P> predicate_, const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        predicate(),
        columns()
    {
        set(predicate_, predicate);
        set(columns_, columns);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::predicate, &Data::columns, &Data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::predicate, &Data::columns> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>> index;
    Index<tyr::formalism::Predicate<T>> predicate {};
    bool polarity {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    Data(Index<tyr::formalism::Predicate<T>> predicate_, bool polarity_, IndexList<runir::kr::dl::QueryColumn> columns_) :
        index(),
        predicate(std::move(predicate_)),
        polarity(polarity_),
        columns(std::move(columns_))
    {
    }
    template<typename C, typename P>
    Data(::ygg::View<Index<tyr::formalism::Predicate<T>>, P> predicate_,
         bool polarity_,
         const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        predicate(),
        polarity(polarity_),
        columns()
    {
        set(predicate_, predicate);
        set(columns_, columns);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::predicate, &Data::polarity, &Data::columns, &Data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::predicate, &Data::polarity, &Data::columns> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>> index;
    Index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    Data(Index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>> arg_, IndexList<runir::kr::dl::QueryColumn> columns_) :
        index(),
        arg(arg_),
        columns(std::move(columns_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>, C> arg_,
         const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        arg(),
        columns()
    {
        set(arg_, arg);
        set(columns_, columns);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::arg, &Data::columns, &Data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::columns> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>> index;
    Index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    Data(Index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>> arg_, IndexList<runir::kr::dl::QueryColumn> columns_) :
        index(),
        arg(arg_),
        columns(std::move(columns_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>, C> arg_,
         const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        arg(),
        columns()
    {
        set(arg_, arg);
        set(columns_, columns);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::arg, &Data::columns, &Data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::columns> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QueryJoinTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QueryJoinTag>> index;
    Index<runir::kr::dl::Query<>> lhs {};
    Index<runir::kr::dl::Query<>> rhs {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::JoinPlan plan {};

    Data() = default;
    // columns and plan are derived during insertion and therefore not constructor parameters.
    Data(Index<runir::kr::dl::Query<>> lhs_, Index<runir::kr::dl::Query<>> rhs_) : index(), lhs(lhs_), rhs(rhs_) {}
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<>>, C> lhs_, ::ygg::View<Index<runir::kr::dl::Query<>>, C> rhs_) : index(), lhs(), rhs()
    {
        set(lhs_, lhs);
        set(rhs_, rhs);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::lhs, &Data::rhs, &Data::columns, &Data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::lhs, &Data::rhs> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QueryUnionTag> || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
struct Data<runir::kr::dl::Query<Tag>>
{
    Index<runir::kr::dl::Query<Tag>> index;
    Index<runir::kr::dl::Query<>> lhs {};
    Index<runir::kr::dl::Query<>> rhs {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};

    Data() = default;
    // columns and schema are derived during insertion and therefore not constructor parameters.
    Data(Index<runir::kr::dl::Query<>> lhs_, Index<runir::kr::dl::Query<>> rhs_) : index(), lhs(lhs_), rhs(rhs_) {}
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<>>, C> lhs_, ::ygg::View<Index<runir::kr::dl::Query<>>, C> rhs_) : index(), lhs(), rhs()
    {
        set(lhs_, lhs);
        set(rhs_, rhs);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::lhs, &Data::rhs, &Data::columns, &Data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::lhs, &Data::rhs> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QueryProjectTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QueryProjectTag>> index;
    Index<runir::kr::dl::Query<>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::ProjectionPlan plan {};

    Data() = default;
    // plan is derived during insertion and therefore not a constructor parameter.
    Data(Index<runir::kr::dl::Query<>> arg_, IndexList<runir::kr::dl::QueryColumn> columns_) : index(), arg(arg_), columns(std::move(columns_)) {}
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<>>, C> arg_, const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        arg(),
        columns()
    {
        set(arg_, arg);
        set(columns_, columns);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::arg, &Data::columns, &Data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::columns> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QueryRenameTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QueryRenameTag>> index;
    Index<runir::kr::dl::Query<>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    Data(Index<runir::kr::dl::Query<>> arg_, IndexList<runir::kr::dl::QueryColumn> columns_) : index(), arg(arg_), columns(std::move(columns_)) {}
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<>>, C> arg_, const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        arg(),
        columns()
    {
        set(arg_, arg);
        set(columns_, columns);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::arg, &Data::columns, &Data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::columns> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>> index;
    Index<runir::kr::dl::Query<>> arg {};
    Index<runir::kr::dl::QueryColumn> lhs_column {};
    Index<runir::kr::dl::QueryColumn> rhs_column {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};
    size_t lhs_position {};
    size_t rhs_position {};

    Data() = default;
    // columns, schema and positions are derived during insertion and therefore not constructor parameters.
    Data(Index<runir::kr::dl::Query<>> arg_, Index<runir::kr::dl::QueryColumn> lhs_column_, Index<runir::kr::dl::QueryColumn> rhs_column_) :
        index(),
        arg(arg_),
        lhs_column(lhs_column_),
        rhs_column(rhs_column_)
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<>>, C> arg_,
         ::ygg::View<Index<runir::kr::dl::QueryColumn>, C> lhs_column_,
         ::ygg::View<Index<runir::kr::dl::QueryColumn>, C> rhs_column_) :
        index(),
        arg(),
        lhs_column(),
        rhs_column()
    {
        set(arg_, arg);
        set(lhs_column_, lhs_column);
        set(rhs_column_, rhs_column);
    }

    static constexpr auto cista_members() noexcept
    {
        return ygg::
            Members<&Data::index, &Data::arg, &Data::lhs_column, &Data::rhs_column, &Data::columns, &Data::schema, &Data::lhs_position, &Data::rhs_position> {};
    }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::lhs_column, &Data::rhs_column> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>> index;
    Index<runir::kr::dl::Query<>> arg {};
    Index<runir::kr::dl::QueryColumn> column {};
    Index<tyr::formalism::Object> object {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};
    size_t position {};

    Data() = default;
    // columns, schema and position are derived during insertion and therefore not constructor parameters.
    Data(Index<runir::kr::dl::Query<>> arg_, Index<runir::kr::dl::QueryColumn> column_, Index<tyr::formalism::Object> object_) :
        index(),
        arg(arg_),
        column(column_),
        object(object_)
    {
    }
    template<typename C, typename P>
    Data(::ygg::View<Index<runir::kr::dl::Query<>>, C> arg_,
         ::ygg::View<Index<runir::kr::dl::QueryColumn>, C> column_,
         ::ygg::View<Index<tyr::formalism::Object>, P> object_) :
        index(),
        arg(),
        column(),
        object()
    {
        set(arg_, arg);
        set(column_, column);
        set(object_, object);
    }

    static constexpr auto cista_members() noexcept
    { return ygg::Members<&Data::index, &Data::arg, &Data::column, &Data::object, &Data::columns, &Data::schema, &Data::position> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::column, &Data::object> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>> index;
    IndexList<runir::kr::dl::Query<>> args {};
    IndexList<runir::kr::dl::QueryColumn> variable_order {};
    IndexList<runir::kr::dl::QueryColumn> output_order {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::GenericJoinPlan plan {};

    Data() = default;
    // columns and plan are derived during insertion and therefore not constructor parameters.
    Data(IndexList<runir::kr::dl::Query<>> args_, IndexList<runir::kr::dl::QueryColumn> variable_order_, IndexList<runir::kr::dl::QueryColumn> output_order_) :
        index(),
        args(std::move(args_)),
        variable_order(std::move(variable_order_)),
        output_order(std::move(output_order_))
    {
    }
    template<typename C>
    Data(const std::vector<::ygg::View<Index<runir::kr::dl::Query<>>, C>>& args_,
         const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& variable_order_,
         const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& output_order_) :
        index(),
        args(),
        variable_order(),
        output_order()
    {
        set(args_, args);
        set(variable_order_, variable_order);
        set(output_order_, output_order);
    }

    static constexpr auto cista_members() noexcept
    { return ygg::Members<&Data::index, &Data::args, &Data::variable_order, &Data::output_order, &Data::columns, &Data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::args, &Data::variable_order, &Data::output_order> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

/// q_must_pass (keys...) sources edges targets probes: per key, the probe objects that every path from the key's sources
/// to its targets passes in some vertex. Sources, edges and targets start with the key columns; probes have one column.
template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QueryMustPassTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QueryMustPassTag>> index;
    IndexList<runir::kr::dl::QueryColumn> keys {};
    Index<runir::kr::dl::Query<>> sources {};
    Index<runir::kr::dl::Query<>> edges {};
    Index<runir::kr::dl::Query<>> targets {};
    Index<runir::kr::dl::Query<>> probes {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};
    ygg::database::MustPassPlan plan {};

    Data() = default;
    // columns, schema and plan are derived during insertion and therefore not constructor parameters.
    Data(IndexList<runir::kr::dl::QueryColumn> keys_,
         Index<runir::kr::dl::Query<>> sources_,
         Index<runir::kr::dl::Query<>> edges_,
         Index<runir::kr::dl::Query<>> targets_,
         Index<runir::kr::dl::Query<>> probes_) :
        index(),
        keys(std::move(keys_)),
        sources(sources_),
        edges(edges_),
        targets(targets_),
        probes(probes_)
    {
    }
    template<typename C>
    Data(const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& keys_,
         ::ygg::View<Index<runir::kr::dl::Query<>>, C> sources_,
         ::ygg::View<Index<runir::kr::dl::Query<>>, C> edges_,
         ::ygg::View<Index<runir::kr::dl::Query<>>, C> targets_,
         ::ygg::View<Index<runir::kr::dl::Query<>>, C> probes_) :
        index(),
        keys(),
        sources(),
        edges(),
        targets(),
        probes()
    {
        set(keys_, keys);
        set(sources_, sources);
        set(edges_, edges);
        set(targets_, targets);
        set(probes_, probes);
    }

    static constexpr auto cista_members() noexcept
    {
        return ygg::
            Members<&Data::index, &Data::keys, &Data::sources, &Data::edges, &Data::targets, &Data::probes, &Data::columns, &Data::schema, &Data::plan> {};
    }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::keys, &Data::sources, &Data::edges, &Data::targets, &Data::probes> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>> index;
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    explicit Data(IndexList<runir::kr::dl::QueryColumn> columns_) : index(), columns(std::move(columns_)) {}
    template<typename C>
    explicit Data(const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) : index(), columns()
    { set(columns_, columns); }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::columns, &Data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::columns> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<runir::kr::dl::ConceptOrRoleTag Category>
struct Data<runir::kr::dl::QueryProjection<Category>>
{
    Index<runir::kr::dl::QueryProjection<Category>> index;
    Index<runir::kr::dl::Query<>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::ProjectionPlan plan {};

    Data() = default;
    // plan is derived during insertion and therefore not a constructor parameter.
    Data(Index<runir::kr::dl::Query<>> arg_, IndexList<runir::kr::dl::QueryColumn> columns_) : index(), arg(arg_), columns(std::move(columns_)) {}
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<>>, C> arg_, const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        arg(),
        columns()
    {
        set(arg_, arg);
        set(columns_, columns);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::arg, &Data::columns, &Data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::columns> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

/// A Datalog program over the repository's DatalogRepository whose input slot s reads the rows of inputs[s].
template<>
struct Data<runir::kr::dl::DatalogProgram>
{
    Index<runir::kr::dl::DatalogProgram> index;
    Index<ygg::database::datalog::Program> program {};
    IndexList<runir::kr::dl::Query<>> inputs {};
    /// The heads that features read (q_head); only these are evaluated, so the others may be evaluated partially.
    IndexList<ygg::database::datalog::Relation> outputs {};
    bool is_static = false;

    Data() = default;
    // is_static is derived during insertion and therefore not a constructor parameter.
    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::program, &Data::inputs, &Data::outputs, &Data::is_static> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::program, &Data::inputs, &Data::outputs> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Query<runir::kr::dl::QueryHeadTag>>
{
    Index<runir::kr::dl::Query<runir::kr::dl::QueryHeadTag>> index;
    Index<runir::kr::dl::DatalogProgram> program {};
    Index<ygg::database::datalog::Relation> relation {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::program, &Data::relation, &Data::columns, &Data::schema> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::program, &Data::relation, &Data::columns> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

}  // namespace ygg

#endif
