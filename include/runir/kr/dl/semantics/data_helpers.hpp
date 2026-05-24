#ifndef RUNIR_SEMANTICS_DATA_HELPERS_HPP_
#define RUNIR_SEMANTICS_DATA_HELPERS_HPP_

#include "runir/kr/dl/indices.hpp"

#include <cista/containers/vector.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/predicate_index.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

template<typename Self>
struct NullaryData
{
    tyr::Index<Self> index;

    NullaryData() = default;

    void clear() noexcept { tyr::clear(index); }

    auto cista_members() const noexcept { return std::tie(index); }
    auto identifying_members() const noexcept { return std::tie(); }
};

template<typename Self, typename Identifier>
struct IdentifierData
{
    tyr::Index<Self> index;
    Identifier identifier;

    IdentifierData() = default;
    explicit IdentifierData(Identifier identifier_) : index(), identifier(std::move(identifier_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(identifier);
    }

    auto cista_members() const noexcept { return std::tie(index, identifier); }
    auto identifying_members() const noexcept { return std::tie(identifier); }
};

template<typename Self, typename Identifier>
struct RegisterData : IdentifierData<Self, Identifier>
{
    using Base = IdentifierData<Self, Identifier>;
    using Base::Base;
};

template<typename Self, typename Identifier>
struct ArgumentData : IdentifierData<Self, Identifier>
{
    using Base = IdentifierData<Self, Identifier>;
    using Base::Base;
};

template<typename Self, typename Arg>
struct UnaryData
{
    tyr::Index<Self> index;
    tyr::Index<Arg> arg;

    UnaryData() = default;
    UnaryData(tyr::Index<Arg> arg_) : index(), arg(std::move(arg_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

template<typename Self, typename Lhs, typename Rhs>
struct BinaryData
{
    tyr::Index<Self> index;
    tyr::Index<Lhs> lhs;
    tyr::Index<Rhs> rhs;

    BinaryData() = default;
    BinaryData(tyr::Index<Lhs> lhs_, tyr::Index<Rhs> rhs_) : index(), lhs(std::move(lhs_)), rhs(std::move(rhs_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(lhs);
        tyr::clear(rhs);
    }

    auto cista_members() const noexcept { return std::tie(index, lhs, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
};

template<typename Self, typename Lhs, typename Mid, typename Rhs>
struct TernaryData
{
    tyr::Index<Self> index;
    tyr::Index<Lhs> lhs;
    tyr::Index<Mid> mid;
    tyr::Index<Rhs> rhs;

    TernaryData() = default;
    TernaryData(tyr::Index<Lhs> lhs_, tyr::Index<Mid> mid_, tyr::Index<Rhs> rhs_) : index(), lhs(std::move(lhs_)), mid(std::move(mid_)), rhs(std::move(rhs_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(lhs);
        tyr::clear(mid);
        tyr::clear(rhs);
    }

    auto cista_members() const noexcept { return std::tie(index, lhs, mid, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, mid, rhs); }
};

template<typename Self, tyr::formalism::FactKind T>
struct PredicateData
{
    tyr::Index<Self> index;
    tyr::Index<tyr::formalism::Predicate<T>> predicate;
    bool polarity;

    PredicateData() = default;
    PredicateData(tyr::Index<tyr::formalism::Predicate<T>> predicate_) : index(), predicate(predicate_), polarity(true) {}
    PredicateData(tyr::Index<tyr::formalism::Predicate<T>> predicate_, bool polarity_) : index(), predicate(predicate_), polarity(polarity_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(predicate);
        tyr::clear(polarity);
    }

    auto cista_members() const noexcept { return std::tie(index, predicate, polarity); }
    auto identifying_members() const noexcept { return std::tie(predicate, polarity); }
};

template<typename Self>
struct ObjectData
{
    tyr::Index<Self> index;
    tyr::Index<tyr::formalism::Object> object;

    ObjectData() = default;
    ObjectData(tyr::Index<tyr::formalism::Object> object_) : index(), object(object_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(object);
    }

    auto cista_members() const noexcept { return std::tie(index, object); }
    auto identifying_members() const noexcept { return std::tie(object); }
};

template<typename Self, typename Role>
struct NumberRestrictionData
{
    tyr::Index<Self> index;
    tyr::uint_t n;
    tyr::Index<Role> role;

    NumberRestrictionData() = default;
    NumberRestrictionData(tyr::uint_t n_, tyr::Index<Role> role_) : index(), n(n_), role(std::move(role_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(n);
        tyr::clear(role);
    }

    auto cista_members() const noexcept { return std::tie(index, n, role); }
    auto identifying_members() const noexcept { return std::tie(n, role); }
};

template<typename Self, typename Role, typename Concept>
struct QualifiedNumberRestrictionData
{
    tyr::Index<Self> index;
    tyr::uint_t n;
    tyr::Index<Role> role;
    tyr::Index<Concept> concept_;

    QualifiedNumberRestrictionData() = default;
    QualifiedNumberRestrictionData(tyr::uint_t n_, tyr::Index<Role> role_, tyr::Index<Concept> concept__) :
        index(),
        n(n_),
        role(std::move(role_)),
        concept_(std::move(concept__))
    {
    }

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(n);
        tyr::clear(role);
        tyr::clear(concept_);
    }

    auto cista_members() const noexcept { return std::tie(index, n, role, concept_); }
    auto identifying_members() const noexcept { return std::tie(n, role, concept_); }
};

template<typename Self, typename Role>
struct RoleFillersData
{
    tyr::Index<Self> index;
    tyr::Index<Role> role;
    tyr::IndexList<tyr::formalism::Object> objects;

    RoleFillersData() = default;
    RoleFillersData(tyr::Index<Role> role_, tyr::IndexList<tyr::formalism::Object> objects_) : index(), role(std::move(role_)), objects(std::move(objects_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(role);
        tyr::clear(objects);
    }

    auto cista_members() const noexcept { return std::tie(index, role, objects); }
    auto identifying_members() const noexcept { return std::tie(role, objects); }
};

template<typename Self>
struct ObjectListData
{
    tyr::Index<Self> index;
    tyr::IndexList<tyr::formalism::Object> objects;

    ObjectListData() = default;
    explicit ObjectListData(tyr::IndexList<tyr::formalism::Object> objects_) : index(), objects(std::move(objects_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(objects);
    }

    auto cista_members() const noexcept { return std::tie(index, objects); }
    auto identifying_members() const noexcept { return std::tie(objects); }
};

}

#endif
