#ifndef RUNIR_CNF_GRAMMAR_CONSTRUCTOR_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_CONSTRUCTOR_DATA_HPP_

#include "runir/cnf_grammar/indices.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Data<runir::cnf_grammar::Constructor<runir::ConceptTag>>
{
    using PrimitiveVariant = ::cista::offset::variant<Index<runir::cnf_grammar::Concept<runir::BotTag>>,
                                                      Index<runir::cnf_grammar::Concept<runir::TopTag>>,
                                                      Index<runir::cnf_grammar::Concept<runir::AtomicStateTag<formalism::StaticTag>>>,
                                                      Index<runir::cnf_grammar::Concept<runir::AtomicStateTag<formalism::FluentTag>>>,
                                                      Index<runir::cnf_grammar::Concept<runir::AtomicStateTag<formalism::DerivedTag>>>,
                                                      Index<runir::cnf_grammar::Concept<runir::AtomicGoalTag<formalism::StaticTag>>>,
                                                      Index<runir::cnf_grammar::Concept<runir::AtomicGoalTag<formalism::FluentTag>>>,
                                                      Index<runir::cnf_grammar::Concept<runir::AtomicGoalTag<formalism::DerivedTag>>>>;

    using CompositeVariant = ::cista::offset::variant<Index<runir::cnf_grammar::Concept<runir::IntersectionTag>>,
                                                      Index<runir::cnf_grammar::Concept<runir::UnionTag>>,
                                                      Index<runir::cnf_grammar::Concept<runir::NegationTag>>,
                                                      Index<runir::cnf_grammar::Concept<runir::ValueRestrictionTag>>,
                                                      Index<runir::cnf_grammar::Concept<runir::ExistentialQuantificationTag>>,
                                                      Index<runir::cnf_grammar::Concept<runir::RoleValueMapContainmentTag>>,
                                                      Index<runir::cnf_grammar::Concept<runir::RoleValueMapEqualityTag>>,
                                                      Index<runir::cnf_grammar::Concept<runir::NominalTag>>>;

    using Variant = ::cista::offset::variant<PrimitiveVariant, CompositeVariant>;

    Variant value;

    Data() = default;
    Data(Variant value_) : value(value_) {}

    void clear() noexcept { tyr::clear(value); }

    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::cnf_grammar::Constructor<runir::RoleTag>>
{
    using PrimitiveVariant = ::cista::offset::variant<Index<runir::cnf_grammar::Role<runir::UniversalTag>>,
                                                      Index<runir::cnf_grammar::Role<runir::AtomicStateTag<formalism::StaticTag>>>,
                                                      Index<runir::cnf_grammar::Role<runir::AtomicStateTag<formalism::FluentTag>>>,
                                                      Index<runir::cnf_grammar::Role<runir::AtomicStateTag<formalism::DerivedTag>>>,
                                                      Index<runir::cnf_grammar::Role<runir::AtomicGoalTag<formalism::StaticTag>>>,
                                                      Index<runir::cnf_grammar::Role<runir::AtomicGoalTag<formalism::FluentTag>>>,
                                                      Index<runir::cnf_grammar::Role<runir::AtomicGoalTag<formalism::DerivedTag>>>,
                                                      Index<runir::cnf_grammar::Role<runir::IntersectionTag>>>;

    using CompositeVariant = ::cista::offset::variant<Index<runir::cnf_grammar::Role<runir::UnionTag>>,
                                                      Index<runir::cnf_grammar::Role<runir::ComplementTag>>,
                                                      Index<runir::cnf_grammar::Role<runir::InverseTag>>,
                                                      Index<runir::cnf_grammar::Role<runir::CompositionTag>>,
                                                      Index<runir::cnf_grammar::Role<runir::TransitiveClosureTag>>,
                                                      Index<runir::cnf_grammar::Role<runir::ReflexiveTransitiveClosureTag>>,
                                                      Index<runir::cnf_grammar::Role<runir::RestrictionTag>>,
                                                      Index<runir::cnf_grammar::Role<runir::IdentityTag>>>;

    using Variant = ::cista::offset::variant<PrimitiveVariant, CompositeVariant>;

    Variant value;

    Data() = default;
    Data(Variant value_) : value(value_) {}

    void clear() noexcept { tyr::clear(value); }

    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::cnf_grammar::Constructor<runir::BooleanTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::cnf_grammar::Boolean<runir::AtomicStateTag<formalism::StaticTag>>>,
                                             Index<runir::cnf_grammar::Boolean<runir::AtomicStateTag<formalism::FluentTag>>>,
                                             Index<runir::cnf_grammar::Boolean<runir::AtomicStateTag<formalism::DerivedTag>>>,
                                             Index<runir::cnf_grammar::Boolean<runir::NonemptyTag>>>;

    Variant value;

    Data() = default;
    Data(Variant value_) : value(value_) {}

    void clear() noexcept { tyr::clear(value); }

    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::cnf_grammar::Constructor<runir::NumericalTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::cnf_grammar::Numerical<runir::CountTag>>, Index<runir::cnf_grammar::Numerical<runir::DistanceTag>>>;

    Variant value;

    Data() = default;
    Data(Variant value_) : value(value_) {}

    void clear() noexcept { tyr::clear(value); }

    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace tyr

#endif
