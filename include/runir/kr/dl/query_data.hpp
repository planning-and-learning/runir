#ifndef RUNIR_KR_DL_QUERY_DATA_HPP_
#define RUNIR_KR_DL_QUERY_DATA_HPP_

#include "runir/kr/dl/indices.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <cstddef>
#include <optional>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/predicate_index.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/database/plans.hpp>

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
    auto identifying_members() const noexcept { return std::tie(name); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family>>
{
    template<typename Tag>
    using AlternativeIndex = ygg::Index<runir::kr::dl::Query<Family, Tag>>;
    using Variant = ygg::ApplyTypeListT<cista::offset::variant, ygg::MapTypeListT<AlternativeIndex, runir::kr::dl::QueryConstructorTags>>;

    Index<runir::kr::dl::Query<Family>> index;
    Variant variant;
    bool is_static = false;

    Data() = default;
    // is_static is derived during insertion and therefore not a constructor parameter.
    explicit Data(Variant variant_) : index(), variant(std::move(variant_)) {}

    auto cista_members() noexcept { return std::tie(index, variant, is_static); }
    auto cista_members() const noexcept { return std::tie(index, variant, is_static); }
    auto identifying_members() const noexcept { return std::tie(variant); }
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

    auto cista_members() noexcept { return std::tie(index, predicate, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, predicate, columns, schema); }
    auto identifying_members() const noexcept { return std::tie(predicate, columns); }
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
    ygg::Builder<ygg::database::Columns> schema {};

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
    auto identifying_members() const noexcept { return std::tie(predicate, polarity, columns); }
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
    ygg::Builder<ygg::database::Columns> schema {};

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
    auto identifying_members() const noexcept { return std::tie(arg, columns); }
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
    ygg::Builder<ygg::database::Columns> schema {};

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
    auto identifying_members() const noexcept { return std::tie(arg, columns); }
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
    ygg::database::JoinPlan plan {};

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
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
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
    ygg::Builder<ygg::database::Columns> schema {};

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
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
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
    ygg::database::ProjectionPlan plan {};

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
    auto identifying_members() const noexcept { return std::tie(arg, columns); }
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
    ygg::Builder<ygg::database::Columns> schema {};

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
    auto identifying_members() const noexcept { return std::tie(arg, columns); }
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
    ygg::Builder<ygg::database::Columns> schema {};
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
    auto identifying_members() const noexcept { return std::tie(arg, lhs_column, rhs_column); }
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
    ygg::Builder<ygg::database::Columns> schema {};
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
    auto identifying_members() const noexcept { return std::tie(arg, column, object); }
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
    ygg::database::ProjectionPlan plan {};

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
    auto identifying_members() const noexcept { return std::tie(arg, columns); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
