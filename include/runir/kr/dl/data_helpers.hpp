#ifndef RUNIR_KR_DL_DATA_HELPERS_HPP_
#define RUNIR_KR_DL_DATA_HELPERS_HPP_

#include "runir/kr/dl/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include <cista/containers/variant.h>
#include <cista/containers/vector.h>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <utility>
#include <vector>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::dl
{

template<typename T>
inline constexpr bool is_cista_variant_v = false;
template<typename... Ts>
inline constexpr bool is_cista_variant_v<urd::flat_variant<Ts...>> = true;

/// How a record stores an operand of type T: an interned record by its index, a variant (a grammar symbol) as it is.
template<typename T>
using Operand = std::conditional_t<is_cista_variant_v<T>, T, urd::index<T>>;

template<typename Self>
struct NullaryData
{
    urd::index<Self> index;

    NullaryData() = default;

    static constexpr auto cista_members() noexcept { return urd::Members<&NullaryData::index> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<> {}; }
};

template<typename Self, typename Identifier>
struct IdentifierData
{
    urd::index<Self> index;
    Identifier identifier {};

    IdentifierData() = default;
    explicit IdentifierData(Identifier identifier_) : index(), identifier(std::move(identifier_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&IdentifierData::index, &IdentifierData::identifier> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&IdentifierData::identifier> {}; }
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
    urd::index<Self> index;
    urd::index<Reference> reference;

    ReferenceData() = default;
    explicit ReferenceData(urd::index<Reference> reference_) : index(), reference(std::move(reference_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&ReferenceData::index, &ReferenceData::reference> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&ReferenceData::reference> {}; }
};

template<typename Self, typename Arg>
struct UnaryData
{
    urd::index<Self> index;
    Operand<Arg> arg;

    UnaryData() = default;
    UnaryData(Operand<Arg> arg_) : index(), arg(std::move(arg_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&UnaryData::index, &UnaryData::arg> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&UnaryData::arg> {}; }
};

template<typename Self, typename Lhs, typename Rhs>
struct BinaryData
{
    urd::index<Self> index;
    Operand<Lhs> lhs;
    Operand<Rhs> rhs;

    BinaryData() = default;
    BinaryData(Operand<Lhs> lhs_, Operand<Rhs> rhs_) : index(), lhs(std::move(lhs_)), rhs(std::move(rhs_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&BinaryData::index, &BinaryData::lhs, &BinaryData::rhs> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&BinaryData::lhs, &BinaryData::rhs> {}; }
};

template<typename Self, typename Lhs, typename Mid, typename Rhs>
struct TernaryData
{
    /// Operand types under the names of the semantic distance record; grammars accept no queries.
    using Vertex = Operand<Lhs>;
    using Edge = Operand<Mid>;

    urd::index<Self> index;
    Operand<Lhs> lhs;
    Operand<Mid> mid;
    Operand<Rhs> rhs;

    TernaryData() = default;
    TernaryData(Operand<Lhs> lhs_, Operand<Mid> mid_, Operand<Rhs> rhs_) : index(), lhs(std::move(lhs_)), mid(std::move(mid_)), rhs(std::move(rhs_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&TernaryData::index, &TernaryData::lhs, &TernaryData::mid, &TernaryData::rhs> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&TernaryData::lhs, &TernaryData::mid, &TernaryData::rhs> {}; }
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
    static constexpr auto identifying_members() noexcept { return urd::Members<&PredicateData::predicate, &PredicateData::polarity> {}; }
};

template<typename Self>
struct ObjectData
{
    urd::index<Self> index;
    urd::index<tyr::formalism::Object> object;

    ObjectData() = default;
    ObjectData(urd::index<tyr::formalism::Object> object_) : index(), object(object_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&ObjectData::index, &ObjectData::object> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&ObjectData::object> {}; }
};

template<typename Self, typename Role>
struct NumberRestrictionData
{
    urd::index<Self> index;
    ygg::uint_t n = 0;
    Operand<Role> role;

    NumberRestrictionData() = default;
    NumberRestrictionData(ygg::uint_t n_, Operand<Role> role_) : index(), n(n_), role(std::move(role_)) {}

    static constexpr auto cista_members() noexcept
    { return urd::Members<&NumberRestrictionData::index, &NumberRestrictionData::n, &NumberRestrictionData::role> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&NumberRestrictionData::n, &NumberRestrictionData::role> {}; }
};

template<typename Self, typename Role, typename Concept>
struct QualifiedNumberRestrictionData
{
    urd::index<Self> index;
    ygg::uint_t n = 0;
    Operand<Role> role;
    Operand<Concept> concept_;

    QualifiedNumberRestrictionData() = default;
    QualifiedNumberRestrictionData(ygg::uint_t n_, Operand<Role> role_, Operand<Concept> concept_arg) :
        index(),
        n(n_),
        role(std::move(role_)),
        concept_(std::move(concept_arg))
    {
    }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&QualifiedNumberRestrictionData::index,
                            &QualifiedNumberRestrictionData::n,
                            &QualifiedNumberRestrictionData::role,
                            &QualifiedNumberRestrictionData::concept_> {};
    }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&QualifiedNumberRestrictionData::n, &QualifiedNumberRestrictionData::role, &QualifiedNumberRestrictionData::concept_> {}; }
};

template<typename Self, typename Role>
struct RoleFillersData
{
    urd::index<Self> index;
    Operand<Role> role;
    urd::flat_vector<urd::index<tyr::formalism::Object>> objects;

    RoleFillersData() = default;
    RoleFillersData(Operand<Role> role_, urd::flat_vector<urd::index<tyr::formalism::Object>> objects_) : index(), role(std::move(role_)), objects(std::move(objects_)) {}
    // Roles and objects live in different repositories, hence the separate context parameters.

    static constexpr auto cista_members() noexcept { return urd::Members<&RoleFillersData::index, &RoleFillersData::role, &RoleFillersData::objects> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&RoleFillersData::role, &RoleFillersData::objects> {}; }
};

template<typename Self>
struct ObjectListData
{
    urd::index<Self> index;
    urd::flat_vector<urd::index<tyr::formalism::Object>> objects;

    ObjectListData() = default;
    explicit ObjectListData(urd::flat_vector<urd::index<tyr::formalism::Object>> objects_) : index(), objects(std::move(objects_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&ObjectListData::index, &ObjectListData::objects> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&ObjectListData::objects> {}; }
};

}

#endif
