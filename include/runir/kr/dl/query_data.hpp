#ifndef RUNIR_KR_DL_QUERY_DATA_HPP_
#define RUNIR_KR_DL_QUERY_DATA_HPP_

#include "runir/kr/dl/indices.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <cstddef>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/predicate_index.hpp>
#include <utility>
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

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(name);
    }

    auto cista_members() const noexcept { return std::tie(index, name); }
    auto identifying_members() const noexcept { return std::tie(name); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family>>
{
    template<typename Tag>
    using AlternativeIndex = ygg::Index<runir::kr::dl::Query<Family, Tag>>;
    using Variant = ygg::ApplyTypeListT<cista::offset::variant, ygg::MapTypeListT<AlternativeIndex, runir::kr::dl::QueryConstructorTags>>;

    Index<runir::kr::dl::Query<Family>> index;
    Variant variant;

    Data() = default;
    explicit Data(Variant variant_) : index(), variant(std::move(variant_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(variant);
    }

    auto cista_members() const noexcept { return std::tie(index, variant); }
    auto identifying_members() const noexcept { return std::tie(variant); }
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::AtomicStateTag<T>>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::AtomicStateTag<T>>> index;
    Index<tyr::formalism::Predicate<T>> predicate {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::Columns schema {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(predicate);
        ygg::clear(columns);
        ygg::clear(schema);
    }

    auto cista_members() noexcept { return std::tie(index, predicate, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, predicate, columns, schema); }
    auto identifying_members() const noexcept { return std::tie(predicate, columns); }
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::AtomicGoalTag<T>>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::AtomicGoalTag<T>>> index;
    Index<tyr::formalism::Predicate<T>> predicate {};
    bool polarity {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::Columns schema {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(predicate);
        ygg::clear(polarity);
        ygg::clear(columns);
        ygg::clear(schema);
    }

    auto cista_members() noexcept { return std::tie(index, predicate, polarity, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, predicate, polarity, columns, schema); }
    auto identifying_members() const noexcept { return std::tie(predicate, polarity, columns); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryConceptTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryConceptTag>> index;
    Index<runir::kr::dl::Constructor<Family, runir::kr::dl::ConceptTag>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::Columns schema {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
        ygg::clear(columns);
        ygg::clear(schema);
    }

    auto cista_members() noexcept { return std::tie(index, arg, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, arg, columns, schema); }
    auto identifying_members() const noexcept { return std::tie(arg, columns); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryRoleTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryRoleTag>> index;
    Index<runir::kr::dl::Constructor<Family, runir::kr::dl::RoleTag>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::Columns schema {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
        ygg::clear(columns);
        ygg::clear(schema);
    }

    auto cista_members() noexcept { return std::tie(index, arg, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, arg, columns, schema); }
    auto identifying_members() const noexcept { return std::tie(arg, columns); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryJoinTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryJoinTag>> index;
    Index<runir::kr::dl::Query<Family>> lhs {};
    Index<runir::kr::dl::Query<Family>> rhs {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::JoinPlan plan {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(lhs);
        ygg::clear(rhs);
        ygg::clear(columns);
        ygg::clear(plan);
    }

    auto cista_members() noexcept { return std::tie(index, lhs, rhs, columns, plan); }
    auto cista_members() const noexcept { return std::tie(index, lhs, rhs, columns, plan); }
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QueryUnionTag> || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
struct Data<runir::kr::dl::Query<Family, Tag>>
{
    Index<runir::kr::dl::Query<Family, Tag>> index;
    Index<runir::kr::dl::Query<Family>> lhs {};
    Index<runir::kr::dl::Query<Family>> rhs {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::Columns schema {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(lhs);
        ygg::clear(rhs);
        ygg::clear(columns);
        ygg::clear(schema);
    }

    auto cista_members() noexcept { return std::tie(index, lhs, rhs, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, lhs, rhs, columns, schema); }
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryProjectTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryProjectTag>> index;
    Index<runir::kr::dl::Query<Family>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::ProjectionPlan plan {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
        ygg::clear(columns);
        ygg::clear(plan);
    }

    auto cista_members() noexcept { return std::tie(index, arg, columns, plan); }
    auto cista_members() const noexcept { return std::tie(index, arg, columns, plan); }
    auto identifying_members() const noexcept { return std::tie(arg, columns); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QueryRenameTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QueryRenameTag>> index;
    Index<runir::kr::dl::Query<Family>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::Columns schema {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
        ygg::clear(columns);
        ygg::clear(schema);
    }

    auto cista_members() noexcept { return std::tie(index, arg, columns, schema); }
    auto cista_members() const noexcept { return std::tie(index, arg, columns, schema); }
    auto identifying_members() const noexcept { return std::tie(arg, columns); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QuerySelectEqualTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QuerySelectEqualTag>> index;
    Index<runir::kr::dl::Query<Family>> arg {};
    Index<runir::kr::dl::QueryColumn> lhs_column {};
    Index<runir::kr::dl::QueryColumn> rhs_column {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::Columns schema {};
    size_t lhs_position {};
    size_t rhs_position {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
        ygg::clear(lhs_column);
        ygg::clear(rhs_column);
        ygg::clear(columns);
        ygg::clear(schema);
        ygg::clear(lhs_position);
        ygg::clear(rhs_position);
    }

    auto cista_members() noexcept { return std::tie(index, arg, lhs_column, rhs_column, columns, schema, lhs_position, rhs_position); }
    auto cista_members() const noexcept { return std::tie(index, arg, lhs_column, rhs_column, columns, schema, lhs_position, rhs_position); }
    auto identifying_members() const noexcept { return std::tie(arg, lhs_column, rhs_column); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Query<Family, runir::kr::dl::QuerySelectValueTag>>
{
    Index<runir::kr::dl::Query<Family, runir::kr::dl::QuerySelectValueTag>> index;
    Index<runir::kr::dl::Query<Family>> arg {};
    Index<runir::kr::dl::QueryColumn> column {};
    Index<tyr::formalism::Object> object {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::Columns schema {};
    size_t position {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
        ygg::clear(column);
        ygg::clear(object);
        ygg::clear(columns);
        ygg::clear(schema);
        ygg::clear(position);
    }

    auto cista_members() noexcept { return std::tie(index, arg, column, object, columns, schema, position); }
    auto cista_members() const noexcept { return std::tie(index, arg, column, object, columns, schema, position); }
    auto identifying_members() const noexcept { return std::tie(arg, column, object); }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::QueryProjection<Family, Category>>
{
    static_assert(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>);

    Index<runir::kr::dl::QueryProjection<Family, Category>> index;
    Index<runir::kr::dl::Query<Family>> arg {};
    IndexList<runir::kr::dl::QueryColumn> columns {};
    ygg::database::ProjectionPlan plan {};

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
        ygg::clear(columns);
        ygg::clear(plan);
    }

    auto cista_members() noexcept { return std::tie(index, arg, columns, plan); }
    auto cista_members() const noexcept { return std::tie(index, arg, columns, plan); }
    auto identifying_members() const noexcept { return std::tie(arg, columns); }
};

}  // namespace ygg

#endif
