#ifndef RUNIR_KR_DL_DATA_HELPERS_HPP_
#define RUNIR_KR_DL_DATA_HELPERS_HPP_

#include "runir/kr/dl/indices.hpp"

#include <cista/containers/vector.h>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/predicate_index.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::dl
{

template<typename Self>
struct NullaryData : ygg::comparison::Mixin<NullaryData<Self>>
{
    ygg::Index<Self> index;

    NullaryData() = default;

    auto cista_members() noexcept { return std::tie(index); }
    auto cista_members() const noexcept { return std::tie(index); }
    auto identifying_members() const noexcept { return std::tie(); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<typename Self, typename Identifier>
struct IdentifierData : ygg::comparison::Mixin<IdentifierData<Self, Identifier>>
{
    ygg::Index<Self> index;
    Identifier identifier;

    IdentifierData() = default;
    explicit IdentifierData(Identifier identifier_) : index(), identifier(std::move(identifier_)) {}

    auto cista_members() noexcept { return std::tie(index, identifier); }
    auto cista_members() const noexcept { return std::tie(index, identifier); }
    auto identifying_members() const noexcept { return std::tie(identifier); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
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
struct ReferenceData : ygg::comparison::Mixin<ReferenceData<Self, Reference>>
{
    ygg::Index<Self> index;
    ygg::Index<Reference> reference;

    ReferenceData() = default;
    explicit ReferenceData(ygg::Index<Reference> reference_) : index(), reference(std::move(reference_)) {}
    template<typename C>
    explicit ReferenceData(::ygg::View<ygg::Index<Reference>, C> reference_) : index(), reference()
    {
        ygg::set(reference_, reference);
    }

    auto cista_members() noexcept { return std::tie(index, reference); }
    auto cista_members() const noexcept { return std::tie(index, reference); }
    auto identifying_members() const noexcept { return std::tie(reference); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<typename Self, typename Arg>
struct UnaryData : ygg::comparison::Mixin<UnaryData<Self, Arg>>
{
    ygg::Index<Self> index;
    ygg::Index<Arg> arg;

    UnaryData() = default;
    UnaryData(ygg::Index<Arg> arg_) : index(), arg(std::move(arg_)) {}
    template<typename C>
    UnaryData(::ygg::View<ygg::Index<Arg>, C> arg_) : index(), arg()
    {
        ygg::set(arg_, arg);
    }

    auto cista_members() noexcept { return std::tie(index, arg); }
    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<typename Self, typename Lhs, typename Rhs>
struct BinaryData : ygg::comparison::Mixin<BinaryData<Self, Lhs, Rhs>>
{
    ygg::Index<Self> index;
    ygg::Index<Lhs> lhs;
    ygg::Index<Rhs> rhs;

    BinaryData() = default;
    BinaryData(ygg::Index<Lhs> lhs_, ygg::Index<Rhs> rhs_) : index(), lhs(std::move(lhs_)), rhs(std::move(rhs_)) {}
    template<typename C>
    BinaryData(::ygg::View<ygg::Index<Lhs>, C> lhs_, ::ygg::View<ygg::Index<Rhs>, C> rhs_) : index(), lhs(), rhs()
    {
        ygg::set(lhs_, lhs);
        ygg::set(rhs_, rhs);
    }

    auto cista_members() noexcept { return std::tie(index, lhs, rhs); }
    auto cista_members() const noexcept { return std::tie(index, lhs, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<typename Self, typename Lhs, typename Mid, typename Rhs>
struct TernaryData : ygg::comparison::Mixin<TernaryData<Self, Lhs, Mid, Rhs>>
{
    ygg::Index<Self> index;
    ygg::Index<Lhs> lhs;
    ygg::Index<Mid> mid;
    ygg::Index<Rhs> rhs;

    TernaryData() = default;
    TernaryData(ygg::Index<Lhs> lhs_, ygg::Index<Mid> mid_, ygg::Index<Rhs> rhs_) : index(), lhs(std::move(lhs_)), mid(std::move(mid_)), rhs(std::move(rhs_)) {}
    template<typename C>
    TernaryData(::ygg::View<ygg::Index<Lhs>, C> lhs_, ::ygg::View<ygg::Index<Mid>, C> mid_, ::ygg::View<ygg::Index<Rhs>, C> rhs_) : index(), lhs(), mid(), rhs()
    {
        ygg::set(lhs_, lhs);
        ygg::set(mid_, mid);
        ygg::set(rhs_, rhs);
    }

    auto cista_members() noexcept { return std::tie(index, lhs, mid, rhs); }
    auto cista_members() const noexcept { return std::tie(index, lhs, mid, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, mid, rhs); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<typename Self, tyr::formalism::FactKind T>
struct PredicateData : ygg::comparison::Mixin<PredicateData<Self, T>>
{
    ygg::Index<Self> index;
    ygg::Index<tyr::formalism::Predicate<T>> predicate;
    bool polarity;

    PredicateData() = default;
    PredicateData(ygg::Index<tyr::formalism::Predicate<T>> predicate_, bool polarity_) : index(), predicate(predicate_), polarity(polarity_) {}
    template<typename C>
    PredicateData(::ygg::View<ygg::Index<tyr::formalism::Predicate<T>>, C> predicate_, bool polarity_) : index(), predicate(), polarity(polarity_)
    {
        ygg::set(predicate_, predicate);
    }

    auto cista_members() noexcept { return std::tie(index, predicate, polarity); }
    auto cista_members() const noexcept { return std::tie(index, predicate, polarity); }
    auto identifying_members() const noexcept { return std::tie(predicate, polarity); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<typename Self>
struct ObjectData : ygg::comparison::Mixin<ObjectData<Self>>
{
    ygg::Index<Self> index;
    ygg::Index<tyr::formalism::Object> object;

    ObjectData() = default;
    ObjectData(ygg::Index<tyr::formalism::Object> object_) : index(), object(object_) {}
    template<typename C>
    ObjectData(::ygg::View<ygg::Index<tyr::formalism::Object>, C> object_) : index(), object()
    {
        ygg::set(object_, object);
    }

    auto cista_members() noexcept { return std::tie(index, object); }
    auto cista_members() const noexcept { return std::tie(index, object); }
    auto identifying_members() const noexcept { return std::tie(object); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<typename Self, typename Role>
struct NumberRestrictionData : ygg::comparison::Mixin<NumberRestrictionData<Self, Role>>
{
    ygg::Index<Self> index;
    ygg::uint_t n;
    ygg::Index<Role> role;

    NumberRestrictionData() = default;
    NumberRestrictionData(ygg::uint_t n_, ygg::Index<Role> role_) : index(), n(n_), role(std::move(role_)) {}
    template<typename C>
    NumberRestrictionData(ygg::uint_t n_, ::ygg::View<ygg::Index<Role>, C> role_) : index(), n(n_), role()
    {
        ygg::set(role_, role);
    }

    auto cista_members() noexcept { return std::tie(index, n, role); }
    auto cista_members() const noexcept { return std::tie(index, n, role); }
    auto identifying_members() const noexcept { return std::tie(n, role); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<typename Self, typename Role, typename Concept>
struct QualifiedNumberRestrictionData : ygg::comparison::Mixin<QualifiedNumberRestrictionData<Self, Role, Concept>>
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
    template<typename C>
    QualifiedNumberRestrictionData(ygg::uint_t n_, ::ygg::View<ygg::Index<Role>, C> role_, ::ygg::View<ygg::Index<Concept>, C> concept__) :
        index(),
        n(n_),
        role(),
        concept_()
    {
        ygg::set(role_, role);
        ygg::set(concept__, concept_);
    }

    auto cista_members() noexcept { return std::tie(index, n, role, concept_); }
    auto cista_members() const noexcept { return std::tie(index, n, role, concept_); }
    auto identifying_members() const noexcept { return std::tie(n, role, concept_); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<typename Self, typename Role>
struct RoleFillersData : ygg::comparison::Mixin<RoleFillersData<Self, Role>>
{
    ygg::Index<Self> index;
    ygg::Index<Role> role;
    ygg::IndexList<tyr::formalism::Object> objects;

    RoleFillersData() = default;
    RoleFillersData(ygg::Index<Role> role_, ygg::IndexList<tyr::formalism::Object> objects_) : index(), role(std::move(role_)), objects(std::move(objects_)) {}
    // Roles and objects live in different repositories, hence the separate context parameters.
    template<typename C, typename P>
    RoleFillersData(::ygg::View<ygg::Index<Role>, C> role_, const std::vector<::ygg::View<ygg::Index<tyr::formalism::Object>, P>>& objects_) :
        index(),
        role(),
        objects()
    {
        ygg::set(role_, role);
        ygg::set(objects_, objects);
    }

    auto cista_members() noexcept { return std::tie(index, role, objects); }
    auto cista_members() const noexcept { return std::tie(index, role, objects); }
    auto identifying_members() const noexcept { return std::tie(role, objects); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<typename Self>
struct ObjectListData : ygg::comparison::Mixin<ObjectListData<Self>>
{
    ygg::Index<Self> index;
    ygg::IndexList<tyr::formalism::Object> objects;

    ObjectListData() = default;
    explicit ObjectListData(ygg::IndexList<tyr::formalism::Object> objects_) : index(), objects(std::move(objects_)) {}
    template<typename C>
    explicit ObjectListData(const std::vector<::ygg::View<ygg::Index<tyr::formalism::Object>, C>>& objects_) : index(), objects()
    {
        ygg::set(objects_, objects);
    }

    auto cista_members() noexcept { return std::tie(index, objects); }
    auto cista_members() const noexcept { return std::tie(index, objects); }
    auto identifying_members() const noexcept { return std::tie(objects); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
