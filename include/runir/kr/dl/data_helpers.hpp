#ifndef RUNIR_KR_DL_DATA_HELPERS_HPP_
#define RUNIR_KR_DL_DATA_HELPERS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <boost/mp11.hpp>
#include <cista/containers/variant.h>
#include <cista/containers/vector.h>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::dl
{

/// The name of a reference field: what it references, a register or an argument.
template<typename Reference>
inline constexpr urd::fixed_string reference_field_name =
    boost::mp11::mp_similar<Reference, Register<ConceptTag>>::value ? urd::fixed_string("register") : urd::fixed_string("argument");

template<typename Self>
struct NullaryData
{
    urd::index<Self> index;

    NullaryData() = default;

    static constexpr auto cista_members() noexcept { return urd::Members<&NullaryData::index> {}; }
    static constexpr auto fields() noexcept { return urd::Fields<> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

template<typename Self, typename Identifier>
struct IdentifierData
{
    urd::index<Self> index;
    Identifier identifier {};

    IdentifierData() = default;
    explicit IdentifierData(Identifier identifier_) : index(), identifier(std::move(identifier_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&IdentifierData::index, &IdentifierData::identifier> {}; }
    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &IdentifierData::identifier>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

template<typename Self, typename Reference>
struct ReferenceData
{
    using Referenced = Reference;

    urd::index<Self> index;
    urd::index<Reference> reference;

    ReferenceData() = default;
    explicit ReferenceData(urd::index<Reference> reference_) : index(), reference(std::move(reference_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&ReferenceData::index, &ReferenceData::reference> {}; }
    static constexpr auto fields() noexcept { return urd::Fields<urd::field<reference_field_name<Reference>, &ReferenceData::reference>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The operand types below are as stored: an index, or a variant of them (a grammar symbol, a semantic collection).
template<typename Self, typename A>
struct UnaryData
{
    using Arg = A;

    urd::index<Self> index;
    Arg arg;

    UnaryData() = default;
    UnaryData(Arg arg_) : index(), arg(std::move(arg_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&UnaryData::index, &UnaryData::arg> {}; }
    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"arg", &UnaryData::arg>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

template<typename Self, typename Lhs, typename Rhs>
struct BinaryData
{
    urd::index<Self> index;
    Lhs lhs;
    Rhs rhs;

    BinaryData() = default;
    BinaryData(Lhs lhs_, Rhs rhs_) : index(), lhs(std::move(lhs_)), rhs(std::move(rhs_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&BinaryData::index, &BinaryData::lhs, &BinaryData::rhs> {}; }
    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"lhs", &BinaryData::lhs>, urd::field<"rhs", &BinaryData::rhs>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// Distance: sources and targets are vertices, the middle operand edges.
template<typename Self, typename V, typename E>
struct TernaryData
{
    using Vertex = V;
    using Edge = E;

    urd::index<Self> index;
    Vertex lhs;
    Edge mid;
    Vertex rhs;

    TernaryData() = default;
    TernaryData(Vertex lhs_, Edge mid_, Vertex rhs_) : index(), lhs(std::move(lhs_)), mid(std::move(mid_)), rhs(std::move(rhs_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&TernaryData::index, &TernaryData::lhs, &TernaryData::mid, &TernaryData::rhs> {}; }
    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"lhs", &TernaryData::lhs>, urd::field<"mid", &TernaryData::mid>, urd::field<"rhs", &TernaryData::rhs>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// A state's atoms of a predicate: no polarity, a negative one is a negation of it.
template<typename Self, tyr::formalism::FactKind T>
struct AtomData
{
    urd::index<Self> index;
    urd::index<tyr::formalism::Predicate<T>> predicate;

    AtomData() = default;
    explicit AtomData(urd::index<tyr::formalism::Predicate<T>> predicate_) : index(), predicate(predicate_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&AtomData::index, &AtomData::predicate> {}; }
    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"predicate", &AtomData::predicate>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

template<typename Self, tyr::formalism::FactKind T>
struct PredicateData
{
    urd::index<Self> index;
    urd::index<tyr::formalism::Predicate<T>> predicate;
    bool polarity = false;

    PredicateData() = default;
    PredicateData(urd::index<tyr::formalism::Predicate<T>> predicate_, bool polarity_) : index(), predicate(predicate_), polarity(polarity_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&PredicateData::index, &PredicateData::predicate, &PredicateData::polarity> {}; }
    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"predicate", &PredicateData::predicate>, urd::field<"polarity", &PredicateData::polarity>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

template<typename Self>
struct ObjectData
{
    urd::index<Self> index;
    urd::index<tyr::formalism::Object> object;

    ObjectData() = default;
    ObjectData(urd::index<tyr::formalism::Object> object_) : index(), object(object_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&ObjectData::index, &ObjectData::object> {}; }
    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"object", &ObjectData::object>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

template<typename Self, typename Role>
struct NumberRestrictionData
{
    urd::index<Self> index;
    ygg::uint_t n = 0;
    Role role;

    NumberRestrictionData() = default;
    NumberRestrictionData(ygg::uint_t n_, Role role_) : index(), n(n_), role(std::move(role_)) {}

    static constexpr auto cista_members() noexcept
    { return urd::Members<&NumberRestrictionData::index, &NumberRestrictionData::n, &NumberRestrictionData::role> {}; }
    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"n", &NumberRestrictionData::n>, urd::field<"role", &NumberRestrictionData::role>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

template<typename Self, typename Role, typename Concept>
struct QualifiedNumberRestrictionData
{
    urd::index<Self> index;
    ygg::uint_t n = 0;
    Role role;
    Concept concept_;

    QualifiedNumberRestrictionData() = default;
    QualifiedNumberRestrictionData(ygg::uint_t n_, Role role_, Concept concept_arg) : index(), n(n_), role(std::move(role_)), concept_(std::move(concept_arg))
    {
    }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&QualifiedNumberRestrictionData::index,
                            &QualifiedNumberRestrictionData::n,
                            &QualifiedNumberRestrictionData::role,
                            &QualifiedNumberRestrictionData::concept_> {};
    }
    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"n", &QualifiedNumberRestrictionData::n>,
                           urd::field<"role", &QualifiedNumberRestrictionData::role>,
                           urd::field<"concept", &QualifiedNumberRestrictionData::concept_>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

template<typename Self, typename Role>
struct RoleFillersData
{
    urd::index<Self> index;
    Role role;
    urd::flat_vector<urd::index<tyr::formalism::Object>> objects;

    RoleFillersData() = default;
    RoleFillersData(Role role_, urd::flat_vector<urd::index<tyr::formalism::Object>> objects_) : index(), role(std::move(role_)), objects(std::move(objects_))
    {
    }
    // Roles and objects live in different repositories, hence the separate context parameters.

    static constexpr auto cista_members() noexcept { return urd::Members<&RoleFillersData::index, &RoleFillersData::role, &RoleFillersData::objects> {}; }
    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"role", &RoleFillersData::role>, urd::field<"objects", &RoleFillersData::objects>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

template<typename Self>
struct ObjectListData
{
    urd::index<Self> index;
    urd::flat_vector<urd::index<tyr::formalism::Object>> objects;

    ObjectListData() = default;
    explicit ObjectListData(urd::flat_vector<urd::index<tyr::formalism::Object>> objects_) : index(), objects(std::move(objects_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&ObjectListData::index, &ObjectListData::objects> {}; }
    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"objects", &ObjectListData::objects>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}

#endif
