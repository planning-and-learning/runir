#ifndef RUNIR_SEMANTICS_DATA_HELPERS_HPP_
#define RUNIR_SEMANTICS_DATA_HELPERS_HPP_

#include "runir/kr/dl/indices.hpp"

#include <cista/containers/vector.h>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/predicate_index.hpp>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace runir::kr::dl::semantics
{

template<typename Self>
struct NullaryData
{
    ygg::Index<Self> index;

    NullaryData() = default;

    void clear() noexcept { ygg::clear(index); }

    auto cista_members() const noexcept { return std::tie(index); }
    auto identifying_members() const noexcept { return std::tie(); }
};

template<typename Self, typename Identifier>
struct IdentifierData
{
    ygg::Index<Self> index;
    Identifier identifier;

    IdentifierData() = default;
    explicit IdentifierData(Identifier identifier_) : index(), identifier(std::move(identifier_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(identifier);
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

template<typename Self, typename Reference>
struct ReferenceData
{
    ygg::Index<Self> index;
    ygg::Index<Reference> reference;

    ReferenceData() = default;
    explicit ReferenceData(ygg::Index<Reference> reference_) : index(), reference(std::move(reference_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(reference);
    }

    auto cista_members() const noexcept { return std::tie(index, reference); }
    auto identifying_members() const noexcept { return std::tie(reference); }
};

template<typename Self, typename Arg>
struct UnaryData
{
    ygg::Index<Self> index;
    ygg::Index<Arg> arg;

    UnaryData() = default;
    UnaryData(ygg::Index<Arg> arg_) : index(), arg(std::move(arg_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

template<typename Self, typename Lhs, typename Rhs>
struct BinaryData
{
    ygg::Index<Self> index;
    ygg::Index<Lhs> lhs;
    ygg::Index<Rhs> rhs;

    BinaryData() = default;
    BinaryData(ygg::Index<Lhs> lhs_, ygg::Index<Rhs> rhs_) : index(), lhs(std::move(lhs_)), rhs(std::move(rhs_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(lhs);
        ygg::clear(rhs);
    }

    auto cista_members() const noexcept { return std::tie(index, lhs, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
};

template<typename Self, typename Lhs, typename Mid, typename Rhs>
struct TernaryData
{
    ygg::Index<Self> index;
    ygg::Index<Lhs> lhs;
    ygg::Index<Mid> mid;
    ygg::Index<Rhs> rhs;

    TernaryData() = default;
    TernaryData(ygg::Index<Lhs> lhs_, ygg::Index<Mid> mid_, ygg::Index<Rhs> rhs_) : index(), lhs(std::move(lhs_)), mid(std::move(mid_)), rhs(std::move(rhs_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(lhs);
        ygg::clear(mid);
        ygg::clear(rhs);
    }

    auto cista_members() const noexcept { return std::tie(index, lhs, mid, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, mid, rhs); }
};

template<typename Self, tyr::formalism::FactKind T>
struct PredicateData
{
    ygg::Index<Self> index;
    ygg::Index<tyr::formalism::Predicate<T>> predicate;
    bool polarity;

    PredicateData() = default;
    PredicateData(ygg::Index<tyr::formalism::Predicate<T>> predicate_) : index(), predicate(predicate_), polarity(true) {}
    PredicateData(ygg::Index<tyr::formalism::Predicate<T>> predicate_, bool polarity_) : index(), predicate(predicate_), polarity(polarity_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(predicate);
        ygg::clear(polarity);
    }

    auto cista_members() const noexcept { return std::tie(index, predicate, polarity); }
    auto identifying_members() const noexcept { return std::tie(predicate, polarity); }
};

template<typename Self>
struct ObjectData
{
    ygg::Index<Self> index;
    ygg::Index<tyr::formalism::Object> object;

    ObjectData() = default;
    ObjectData(ygg::Index<tyr::formalism::Object> object_) : index(), object(object_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(object);
    }

    auto cista_members() const noexcept { return std::tie(index, object); }
    auto identifying_members() const noexcept { return std::tie(object); }
};

template<typename Self, typename Role>
struct NumberRestrictionData
{
    ygg::Index<Self> index;
    ygg::uint_t n;
    ygg::Index<Role> role;

    NumberRestrictionData() = default;
    NumberRestrictionData(ygg::uint_t n_, ygg::Index<Role> role_) : index(), n(n_), role(std::move(role_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(n);
        ygg::clear(role);
    }

    auto cista_members() const noexcept { return std::tie(index, n, role); }
    auto identifying_members() const noexcept { return std::tie(n, role); }
};

template<typename Self, typename Role, typename Concept>
struct QualifiedNumberRestrictionData
{
    ygg::Index<Self> index;
    ygg::uint_t n;
    ygg::Index<Role> role;
    ygg::Index<Concept> concept_;

    QualifiedNumberRestrictionData() = default;
    QualifiedNumberRestrictionData(ygg::uint_t n_, ygg::Index<Role> role_, ygg::Index<Concept> concept__) :
        index(),
        n(n_),
        role(std::move(role_)),
        concept_(std::move(concept__))
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(n);
        ygg::clear(role);
        ygg::clear(concept_);
    }

    auto cista_members() const noexcept { return std::tie(index, n, role, concept_); }
    auto identifying_members() const noexcept { return std::tie(n, role, concept_); }
};

template<typename Self, typename Role>
struct RoleFillersData
{
    ygg::Index<Self> index;
    ygg::Index<Role> role;
    ygg::IndexList<tyr::formalism::Object> objects;

    RoleFillersData() = default;
    RoleFillersData(ygg::Index<Role> role_, ygg::IndexList<tyr::formalism::Object> objects_) : index(), role(std::move(role_)), objects(std::move(objects_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(role);
        ygg::clear(objects);
    }

    auto cista_members() const noexcept { return std::tie(index, role, objects); }
    auto identifying_members() const noexcept { return std::tie(role, objects); }
};

template<typename Self>
struct ObjectListData
{
    ygg::Index<Self> index;
    ygg::IndexList<tyr::formalism::Object> objects;

    ObjectListData() = default;
    explicit ObjectListData(ygg::IndexList<tyr::formalism::Object> objects_) : index(), objects(std::move(objects_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(objects);
    }

    auto cista_members() const noexcept { return std::tie(index, objects); }
    auto identifying_members() const noexcept { return std::tie(objects); }
};

}

#endif
