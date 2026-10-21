#ifndef RUNIR_KR_DL_QUERY_DATA_HPP_
#define RUNIR_KR_DL_QUERY_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
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
#include <yggdrasil/database/semantics/generic_join.hpp>
#include <yggdrasil/database/semantics/must_pass_plan.hpp>
#include <yggdrasil/database/semantics/plans.hpp>

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

    auto cista_members() noexcept { return std::tie(index, name); }
    auto cista_members() const noexcept { return std::tie(index, name); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::name> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family>>
{
private:
    template<typename Tag>
    using AlternativeIndex = ygg::Index<runir::kr::dl::Query<Family, Tag>>;

public:
    using Variant = ygg::ApplyTypeListT<cista::offset::variant, ygg::MapTypeListT<AlternativeIndex, runir::kr::dl::QueryConstructorTags>>;

    Index<runir::kr::dl::Query<Family>> index;
    Variant variant;
    bool is_static = false;

    Data() = default;
    // is_static is derived during insertion and therefore not a constructor parameter.
    explicit Data(Variant variant_) : index(), variant(std::move(variant_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<Variant, C>;
    template<typename C>
    explicit Data(const ViewVariant<C>& variant_) : index(), variant(std::visit([](const auto& view) -> Variant { return Variant(view.get_index()); }, variant_))
    {
    }

    auto cista_members() noexcept { return std::tie(index, variant, is_static); }
    auto cista_members() const noexcept { return std::tie(index, variant, is_static); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::variant> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::AtomicStateTag<T>>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::AtomicStateTag<T>>> index;
    Index<tyr::formalism::Predicate<T>> predicate {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns<runir::kr::dl::QueryValues>> schema {};

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

    auto cista_members() noexcept { return std::tie(index, predicate, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, predicate, columns, schema); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::predicate, &Data::columns> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::AtomicGoalTag<T>>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::AtomicGoalTag<T>>> index;
    Index<tyr::formalism::Predicate<T>> predicate {};
    bool polarity {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns<runir::kr::dl::QueryValues>> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    Data(Index<tyr::formalism::Predicate<T>> predicate_, bool polarity_, IndexList<runir::kr::dl::QueryColumn> columns_) :
        index(),
        predicate(std::move(predicate_)),
        polarity(std::move(polarity_)),
        columns(std::move(columns_))
    {
    }
    template<typename C, typename P>
    Data(::ygg::View<Index<tyr::formalism::Predicate<T>>, P> predicate_,
         bool polarity_,
         const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        predicate(),
        polarity(std::move(polarity_)),
        columns()
    {
        set(predicate_, predicate);
        set(columns_, columns);
    }

    auto cista_members() noexcept { return std::tie(index, predicate, polarity, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, predicate, polarity, columns, schema); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::predicate, &Data::polarity, &Data::columns> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryConceptTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryConceptTag>> index;
    Index<runir::kr::dl::Constructor<Family, runir::kr::dl::ConceptTag>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns<runir::kr::dl::QueryValues>> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    Data(Index<runir::kr::dl::Constructor<Family, runir::kr::dl::ConceptTag>> arg_, IndexList<runir::kr::dl::QueryColumn> columns_) :
        index(),
        arg(std::move(arg_)),
        columns(std::move(columns_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Constructor<Family, runir::kr::dl::ConceptTag>>, C> arg_,
         const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        arg(),
        columns()
    {
        set(arg_, arg);
        set(columns_, columns);
    }

    auto cista_members() noexcept { return std::tie(index, arg, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, arg, columns, schema); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::columns> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryRoleTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryRoleTag>> index;
    Index<runir::kr::dl::Constructor<Family, runir::kr::dl::RoleTag>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns<runir::kr::dl::QueryValues>> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    Data(Index<runir::kr::dl::Constructor<Family, runir::kr::dl::RoleTag>> arg_, IndexList<runir::kr::dl::QueryColumn> columns_) :
        index(),
        arg(std::move(arg_)),
        columns(std::move(columns_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Constructor<Family, runir::kr::dl::RoleTag>>, C> arg_,
         const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        arg(),
        columns()
    {
        set(arg_, arg);
        set(columns_, columns);
    }

    auto cista_members() noexcept { return std::tie(index, arg, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, arg, columns, schema); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::columns> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryJoinTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryJoinTag>> index;
    Index<runir::kr::dl::Query<Family>> lhs {};
    Index<runir::kr::dl::Query<Family>> rhs {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::JoinPlan<runir::kr::dl::QueryValues> plan {};

    Data() = default;
    // columns and plan are derived during insertion and therefore not constructor parameters.
    Data(Index<runir::kr::dl::Query<Family>> lhs_, Index<runir::kr::dl::Query<Family>> rhs_) : index(), lhs(std::move(lhs_)), rhs(std::move(rhs_)) {}
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<Family>>, C> lhs_, ::ygg::View<Index<runir::kr::dl::Query<Family>>, C> rhs_) : index(), lhs(), rhs()
    {
        set(lhs_, lhs);
        set(rhs_, rhs);
    }

    auto cista_members() noexcept { return std::tie(index, lhs, rhs, columns, plan); }
    auto cista_members() const noexcept { return std::tie(index, lhs, rhs, columns, plan); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::lhs, &Data::rhs> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QueryUnionTag> || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
struct Data<runir::kr::dl::Query<Family, Tag>>
{
    Index<runir::kr::dl::Query<Family, Tag>> index;
    Index<runir::kr::dl::Query<Family>> lhs {};
    Index<runir::kr::dl::Query<Family>> rhs {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns<runir::kr::dl::QueryValues>> schema {};

    Data() = default;
    // columns and schema are derived during insertion and therefore not constructor parameters.
    Data(Index<runir::kr::dl::Query<Family>> lhs_, Index<runir::kr::dl::Query<Family>> rhs_) : index(), lhs(std::move(lhs_)), rhs(std::move(rhs_)) {}
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<Family>>, C> lhs_, ::ygg::View<Index<runir::kr::dl::Query<Family>>, C> rhs_) : index(), lhs(), rhs()
    {
        set(lhs_, lhs);
        set(rhs_, rhs);
    }

    auto cista_members() noexcept { return std::tie(index, lhs, rhs, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, lhs, rhs, columns, schema); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::lhs, &Data::rhs> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryProjectTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryProjectTag>> index;
    Index<runir::kr::dl::Query<Family>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::ProjectionPlan<runir::kr::dl::QueryValues> plan {};

    Data() = default;
    // plan is derived during insertion and therefore not a constructor parameter.
    Data(Index<runir::kr::dl::Query<Family>> arg_, IndexList<runir::kr::dl::QueryColumn> columns_) : index(), arg(std::move(arg_)), columns(std::move(columns_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<Family>>, C> arg_, const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        arg(),
        columns()
    {
        set(arg_, arg);
        set(columns_, columns);
    }

    auto cista_members() noexcept { return std::tie(index, arg, columns, plan); }
    auto cista_members() const noexcept { return std::tie(index, arg, columns, plan); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::columns> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryRenameTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryRenameTag>> index;
    Index<runir::kr::dl::Query<Family>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns<runir::kr::dl::QueryValues>> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    Data(Index<runir::kr::dl::Query<Family>> arg_, IndexList<runir::kr::dl::QueryColumn> columns_) : index(), arg(std::move(arg_)), columns(std::move(columns_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<Family>>, C> arg_, const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        arg(),
        columns()
    {
        set(arg_, arg);
        set(columns_, columns);
    }

    auto cista_members() noexcept { return std::tie(index, arg, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, arg, columns, schema); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::columns> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QuerySelectEqualTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QuerySelectEqualTag>> index;
    Index<runir::kr::dl::Query<Family>> arg {};
    Index<runir::kr::dl::QueryColumn> lhs_column {};
    Index<runir::kr::dl::QueryColumn> rhs_column {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns<runir::kr::dl::QueryValues>> schema {};
    size_t lhs_position {};
    size_t rhs_position {};

    Data() = default;
    // columns, schema and positions are derived during insertion and therefore not constructor parameters.
    Data(Index<runir::kr::dl::Query<Family>> arg_, Index<runir::kr::dl::QueryColumn> lhs_column_, Index<runir::kr::dl::QueryColumn> rhs_column_) :
        index(),
        arg(std::move(arg_)),
        lhs_column(std::move(lhs_column_)),
        rhs_column(std::move(rhs_column_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<Family>>, C> arg_,
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

    auto cista_members() noexcept { return std::tie(index, arg, lhs_column, rhs_column, columns, schema, lhs_position, rhs_position); }
    auto cista_members() const noexcept { return std::tie(index, arg, lhs_column, rhs_column, columns, schema, lhs_position, rhs_position); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::lhs_column, &Data::rhs_column> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QuerySelectValueTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QuerySelectValueTag>> index;
    Index<runir::kr::dl::Query<Family>> arg {};
    Index<runir::kr::dl::QueryColumn> column {};
    Index<tyr::formalism::Object> object {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns<runir::kr::dl::QueryValues>> schema {};
    size_t position {};

    Data() = default;
    // columns, schema and position are derived during insertion and therefore not constructor parameters.
    Data(Index<runir::kr::dl::Query<Family>> arg_, Index<runir::kr::dl::QueryColumn> column_, Index<tyr::formalism::Object> object_) :
        index(),
        arg(std::move(arg_)),
        column(std::move(column_)),
        object(std::move(object_))
    {
    }
    template<typename C, typename P>
    Data(::ygg::View<Index<runir::kr::dl::Query<Family>>, C> arg_,
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

    auto cista_members() noexcept { return std::tie(index, arg, column, object, columns, schema, position); }
    auto cista_members() const noexcept { return std::tie(index, arg, column, object, columns, schema, position); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::column, &Data::object> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryGenericJoinTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryGenericJoinTag>> index;
    IndexList<runir::kr::dl::Query<Family>> args {};
    IndexList<runir::kr::dl::QueryColumn> variable_order {};
    IndexList<runir::kr::dl::QueryColumn> output_order {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::GenericJoinPlan<runir::kr::dl::QueryValues> plan {};

    Data() = default;
    // columns and plan are derived during insertion and therefore not constructor parameters.
    Data(IndexList<runir::kr::dl::Query<Family>> args_,
         IndexList<runir::kr::dl::QueryColumn> variable_order_,
         IndexList<runir::kr::dl::QueryColumn> output_order_) :
        index(),
        args(std::move(args_)),
        variable_order(std::move(variable_order_)),
        output_order(std::move(output_order_))
    {
    }
    template<typename C>
    Data(const std::vector<::ygg::View<Index<runir::kr::dl::Query<Family>>, C>>& args_,
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

    auto cista_members() noexcept { return std::tie(index, args, variable_order, output_order, columns, plan); }
    auto cista_members() const noexcept { return std::tie(index, args, variable_order, output_order, columns, plan); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::args, &Data::variable_order, &Data::output_order> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

/// q_must_pass (keys...) sources edges targets probes: per key, the probe objects that every path from the key's sources
/// to its targets passes in some vertex. Sources, edges and targets start with the key columns; probes have one column.
template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryMustPassTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryMustPassTag>> index;
    IndexList<runir::kr::dl::QueryColumn> keys {};
    Index<runir::kr::dl::Query<Family>> sources {};
    Index<runir::kr::dl::Query<Family>> edges {};
    Index<runir::kr::dl::Query<Family>> targets {};
    Index<runir::kr::dl::Query<Family>> probes {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns<runir::kr::dl::QueryValues>> schema {};
    ygg::database::MustPassPlan<runir::kr::dl::QueryValues> plan {};

    Data() = default;
    // columns, schema and plan are derived during insertion and therefore not constructor parameters.
    Data(IndexList<runir::kr::dl::QueryColumn> keys_,
         Index<runir::kr::dl::Query<Family>> sources_,
         Index<runir::kr::dl::Query<Family>> edges_,
         Index<runir::kr::dl::Query<Family>> targets_,
         Index<runir::kr::dl::Query<Family>> probes_) :
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
         ::ygg::View<Index<runir::kr::dl::Query<Family>>, C> sources_,
         ::ygg::View<Index<runir::kr::dl::Query<Family>>, C> edges_,
         ::ygg::View<Index<runir::kr::dl::Query<Family>>, C> targets_,
         ::ygg::View<Index<runir::kr::dl::Query<Family>>, C> probes_) :
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

    auto cista_members() noexcept { return std::tie(index, keys, sources, edges, targets, probes, columns, schema, plan); }
    auto cista_members() const noexcept { return std::tie(index, keys, sources, edges, targets, probes, columns, schema, plan); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::keys, &Data::sources, &Data::edges, &Data::targets, &Data::probes> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryEmptyTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryEmptyTag>> index;
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::Builder<ygg::database::Columns<runir::kr::dl::QueryValues>> schema {};

    Data() = default;
    // schema is derived during insertion and therefore not a constructor parameter.
    explicit Data(IndexList<runir::kr::dl::QueryColumn> columns_) : index(), columns(std::move(columns_)) {}
    template<typename C>
    explicit Data(const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) : index(), columns()
    {
        set(columns_, columns);
    }

    auto cista_members() noexcept { return std::tie(index, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, columns, schema); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::columns> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::ConceptOrRoleTag Category>
struct Data<runir::kr::dl::QueryProjection<Family, Category>>
{
    Index<runir::kr::dl::QueryProjection<Family, Category>> index;
    Index<runir::kr::dl::Query<Family>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::ProjectionPlan<runir::kr::dl::QueryValues> plan {};

    Data() = default;
    // plan is derived during insertion and therefore not a constructor parameter.
    Data(Index<runir::kr::dl::Query<Family>> arg_, IndexList<runir::kr::dl::QueryColumn> columns_) : index(), arg(std::move(arg_)), columns(std::move(columns_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::Query<Family>>, C> arg_, const std::vector<::ygg::View<Index<runir::kr::dl::QueryColumn>, C>>& columns_) :
        index(),
        arg(),
        columns()
    {
        set(arg_, arg);
        set(columns_, columns);
    }

    auto cista_members() noexcept { return std::tie(index, arg, columns, plan); }
    auto cista_members() const noexcept { return std::tie(index, arg, columns, plan); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg, &Data::columns> {}; }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
