#ifndef RUNIR_SEMANTICS_CONSTRUCTOR_DATA_HPP_
#define RUNIR_SEMANTICS_CONSTRUCTOR_DATA_HPP_

#include "runir/indices.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Data<runir::Constructor<runir::ConceptTag>>
{
    using PrimitiveVariant = ::cista::offset::variant<Index<runir::Concept<runir::BotTag>>,
                                                      Index<runir::Concept<runir::TopTag>>,
                                                      Index<runir::Concept<runir::AtomicStateTag<tyr::formalism::StaticTag>>>,
                                                      Index<runir::Concept<runir::AtomicStateTag<tyr::formalism::FluentTag>>>,
                                                      Index<runir::Concept<runir::AtomicStateTag<tyr::formalism::DerivedTag>>>,
                                                      Index<runir::Concept<runir::AtomicGoalTag<tyr::formalism::StaticTag>>>,
                                                      Index<runir::Concept<runir::AtomicGoalTag<tyr::formalism::FluentTag>>>,
                                                      Index<runir::Concept<runir::AtomicGoalTag<tyr::formalism::DerivedTag>>>>;

    using CompositeVariant = ::cista::offset::variant<Index<runir::Concept<runir::IntersectionTag>>,
                                                      Index<runir::Concept<runir::UnionTag>>,
                                                      Index<runir::Concept<runir::NegationTag>>,
                                                      Index<runir::Concept<runir::ValueRestrictionTag>>,
                                                      Index<runir::Concept<runir::ExistentialQuantificationTag>>,
                                                      Index<runir::Concept<runir::RoleValueMapContainmentTag>>,
                                                      Index<runir::Concept<runir::RoleValueMapEqualityTag>>,
                                                      Index<runir::Concept<runir::NominalTag>>>;

    using Variant = ::cista::offset::variant<PrimitiveVariant, CompositeVariant>;

    Variant value;

    Data() = default;
    Data(Variant value_) : value(value_) {}

    void clear() noexcept { tyr::clear(value); }

    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::Constructor<runir::RoleTag>>
{
    using PrimitiveVariant = ::cista::offset::variant<Index<runir::Role<runir::UniversalTag>>,
                                                      Index<runir::Role<runir::AtomicStateTag<tyr::formalism::StaticTag>>>,
                                                      Index<runir::Role<runir::AtomicStateTag<tyr::formalism::FluentTag>>>,
                                                      Index<runir::Role<runir::AtomicStateTag<tyr::formalism::DerivedTag>>>,
                                                      Index<runir::Role<runir::AtomicGoalTag<tyr::formalism::StaticTag>>>,
                                                      Index<runir::Role<runir::AtomicGoalTag<tyr::formalism::FluentTag>>>,
                                                      Index<runir::Role<runir::AtomicGoalTag<tyr::formalism::DerivedTag>>>,
                                                      Index<runir::Role<runir::IntersectionTag>>>;

    using CompositeVariant = ::cista::offset::variant<Index<runir::Role<runir::UnionTag>>,
                                                      Index<runir::Role<runir::ComplementTag>>,
                                                      Index<runir::Role<runir::InverseTag>>,
                                                      Index<runir::Role<runir::CompositionTag>>,
                                                      Index<runir::Role<runir::TransitiveClosureTag>>,
                                                      Index<runir::Role<runir::ReflexiveTransitiveClosureTag>>,
                                                      Index<runir::Role<runir::RestrictionTag>>,
                                                      Index<runir::Role<runir::IdentityTag>>>;

    using Variant = ::cista::offset::variant<PrimitiveVariant, CompositeVariant>;

    Variant value;

    Data() = default;
    Data(Variant value_) : value(value_) {}

    void clear() noexcept { tyr::clear(value); }

    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::Constructor<runir::BooleanTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::Boolean<runir::AtomicStateTag<tyr::formalism::StaticTag>>>,
                                             Index<runir::Boolean<runir::AtomicStateTag<tyr::formalism::FluentTag>>>,
                                             Index<runir::Boolean<runir::AtomicStateTag<tyr::formalism::DerivedTag>>>,
                                             Index<runir::Boolean<runir::NonemptyTag>>>;

    Variant value;

    Data() = default;
    Data(Variant value_) : value(value_) {}

    void clear() noexcept { tyr::clear(value); }

    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::Constructor<runir::NumericalTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::Numerical<runir::CountTag>>, Index<runir::Numerical<runir::DistanceTag>>>;

    Variant value;

    Data() = default;
    Data(Variant value_) : value(value_) {}

    void clear() noexcept { tyr::clear(value); }

    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}

#endif
