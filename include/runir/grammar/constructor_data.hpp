#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_DATA_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_DATA_HPP_

#include "runir/grammar/indices.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <variant>

namespace tyr
{

template<>
struct Data<runir::grammar::Constructor<runir::ConceptTag>>
{
    using Variant = std::variant<Index<runir::grammar::Concept<runir::BotTag>>,
                                 Index<runir::grammar::Concept<runir::TopTag>>,
                                 Index<runir::grammar::Concept<runir::AtomicStateTag<formalism::StaticTag>>>,
                                 Index<runir::grammar::Concept<runir::AtomicStateTag<formalism::FluentTag>>>,
                                 Index<runir::grammar::Concept<runir::AtomicStateTag<formalism::DerivedTag>>>,
                                 Index<runir::grammar::Concept<runir::AtomicGoalTag<formalism::StaticTag>>>,
                                 Index<runir::grammar::Concept<runir::AtomicGoalTag<formalism::FluentTag>>>,
                                 Index<runir::grammar::Concept<runir::AtomicGoalTag<formalism::DerivedTag>>>,
                                 Index<runir::grammar::Concept<runir::IntersectionTag>>,
                                 Index<runir::grammar::Concept<runir::UnionTag>>,
                                 Index<runir::grammar::Concept<runir::NegationTag>>,
                                 Index<runir::grammar::Concept<runir::ValueRestrictionTag>>,
                                 Index<runir::grammar::Concept<runir::ExistentialQuantificationTag>>,
                                 Index<runir::grammar::Concept<runir::RoleValueMapContainmentTag>>,
                                 Index<runir::grammar::Concept<runir::RoleValueMapEqualityTag>>,
                                 Index<runir::grammar::Concept<runir::NominalTag>>>;
    Variant value;
    Data() = default;
    Data(Variant value_) : value(value_) {}
    void clear() noexcept { value = Variant(); }
    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::grammar::Constructor<runir::RoleTag>>
{
    using Variant = std::variant<Index<runir::grammar::Role<runir::UniversalTag>>,
                                 Index<runir::grammar::Role<runir::AtomicStateTag<formalism::StaticTag>>>,
                                 Index<runir::grammar::Role<runir::AtomicStateTag<formalism::FluentTag>>>,
                                 Index<runir::grammar::Role<runir::AtomicStateTag<formalism::DerivedTag>>>,
                                 Index<runir::grammar::Role<runir::AtomicGoalTag<formalism::StaticTag>>>,
                                 Index<runir::grammar::Role<runir::AtomicGoalTag<formalism::FluentTag>>>,
                                 Index<runir::grammar::Role<runir::AtomicGoalTag<formalism::DerivedTag>>>,
                                 Index<runir::grammar::Role<runir::IntersectionTag>>,
                                 Index<runir::grammar::Role<runir::UnionTag>>,
                                 Index<runir::grammar::Role<runir::ComplementTag>>,
                                 Index<runir::grammar::Role<runir::InverseTag>>,
                                 Index<runir::grammar::Role<runir::CompositionTag>>,
                                 Index<runir::grammar::Role<runir::TransitiveClosureTag>>,
                                 Index<runir::grammar::Role<runir::ReflexiveTransitiveClosureTag>>,
                                 Index<runir::grammar::Role<runir::RestrictionTag>>,
                                 Index<runir::grammar::Role<runir::IdentityTag>>>;
    Variant value;
    Data() = default;
    Data(Variant value_) : value(value_) {}
    void clear() noexcept { value = Variant(); }
    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::grammar::Constructor<runir::BooleanTag>>
{
    using Variant = std::variant<Index<runir::grammar::Boolean<runir::AtomicStateTag<formalism::StaticTag>>>,
                                 Index<runir::grammar::Boolean<runir::AtomicStateTag<formalism::FluentTag>>>,
                                 Index<runir::grammar::Boolean<runir::AtomicStateTag<formalism::DerivedTag>>>,
                                 Index<runir::grammar::Boolean<runir::NonemptyTag>>>;
    Variant value;
    Data() = default;
    Data(Variant value_) : value(value_) {}
    void clear() noexcept { value = Variant(); }
    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::grammar::Constructor<runir::NumericalTag>>
{
    using Variant = std::variant<Index<runir::grammar::Numerical<runir::CountTag>>, Index<runir::grammar::Numerical<runir::DistanceTag>>>;
    Variant value;
    Data() = default;
    Data(Variant value_) : value(value_) {}
    void clear() noexcept { value = Variant(); }
    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace tyr

#endif
