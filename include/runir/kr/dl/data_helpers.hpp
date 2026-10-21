#ifndef RUNIR_KR_DL_DATA_HELPERS_HPP_
#define RUNIR_KR_DL_DATA_HELPERS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cista/containers/vector.h>
#include <tuple>
#include <tyr/formalism/declarations.hpp>
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

    static constexpr auto cista_members() noexcept { return ygg::Members<&NullaryData::index> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

template<typename Self, typename Identifier>
struct IdentifierData : ygg::comparison::Mixin<IdentifierData<Self, Identifier>>
{
    ygg::Index<Self> index;
    Identifier identifier;

    IdentifierData() = default;
    explicit IdentifierData(Identifier identifier_) : index(), identifier(std::move(identifier_)) {}

    static constexpr auto cista_members() noexcept { return ygg::Members<&IdentifierData::index, &IdentifierData::identifier> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&IdentifierData::identifier> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
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

    static constexpr auto cista_members() noexcept { return ygg::Members<&ReferenceData::index, &ReferenceData::reference> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&ReferenceData::reference> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
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

    static constexpr auto cista_members() noexcept { return ygg::Members<&UnaryData::index, &UnaryData::arg> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&UnaryData::arg> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
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

    static constexpr auto cista_members() noexcept { return ygg::Members<&BinaryData::index, &BinaryData::lhs, &BinaryData::rhs> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&BinaryData::lhs, &BinaryData::rhs> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

template<typename Self, typename Lhs, typename Mid, typename Rhs>
struct TernaryData : ygg::comparison::Mixin<TernaryData<Self, Lhs, Mid, Rhs>>
{
    /// Operand types under the names of the semantic distance record; grammars accept no queries.
    using Vertex = ygg::Index<Lhs>;
    using Edge = ygg::Index<Mid>;

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

    static constexpr auto cista_members() noexcept { return ygg::Members<&TernaryData::index, &TernaryData::lhs, &TernaryData::mid, &TernaryData::rhs> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&TernaryData::lhs, &TernaryData::mid, &TernaryData::rhs> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
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

    static constexpr auto cista_members() noexcept { return ygg::Members<&PredicateData::index, &PredicateData::predicate, &PredicateData::polarity> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&PredicateData::predicate, &PredicateData::polarity> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
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

    static constexpr auto cista_members() noexcept { return ygg::Members<&ObjectData::index, &ObjectData::object> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&ObjectData::object> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
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

    static constexpr auto cista_members() noexcept { return ygg::Members<&NumberRestrictionData::index, &NumberRestrictionData::n, &NumberRestrictionData::role> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&NumberRestrictionData::n, &NumberRestrictionData::role> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
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

    static constexpr auto cista_members() noexcept { return ygg::Members<&QualifiedNumberRestrictionData::index, &QualifiedNumberRestrictionData::n, &QualifiedNumberRestrictionData::role, &QualifiedNumberRestrictionData::concept_> {}; }
    static constexpr auto identifying_members() noexcept
    { return ygg::Members<&QualifiedNumberRestrictionData::n, &QualifiedNumberRestrictionData::role, &QualifiedNumberRestrictionData::concept_> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
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

    static constexpr auto cista_members() noexcept { return ygg::Members<&RoleFillersData::index, &RoleFillersData::role, &RoleFillersData::objects> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&RoleFillersData::role, &RoleFillersData::objects> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
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

    static constexpr auto cista_members() noexcept { return ygg::Members<&ObjectListData::index, &ObjectListData::objects> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&ObjectListData::objects> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

}

#endif
