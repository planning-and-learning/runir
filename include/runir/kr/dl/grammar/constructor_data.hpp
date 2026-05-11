#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_DATA_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_DATA_HPP_

#include "runir/kr/dl/grammar/indices.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::dl::grammar::Constructor<runir::kr::dl::ConceptTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BotTag>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::TopTag>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<formalism::StaticTag>>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<formalism::FluentTag>>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<formalism::StaticTag>>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<formalism::FluentTag>>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::IntersectionTag>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::UnionTag>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::NegationTag>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::ValueRestrictionTag>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::ExistentialQuantificationTag>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleValueMapContainmentTag>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleValueMapEqualityTag>>,
                                             Index<runir::kr::dl::grammar::Concept<runir::kr::dl::NominalTag>>>;
    Index<runir::kr::dl::grammar::Constructor<runir::kr::dl::ConceptTag>> index;
    Variant value;
    Data() = default;
    Data(Variant value_) : value(value_) {}
    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::kr::dl::grammar::Constructor<runir::kr::dl::RoleTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::grammar::Role<runir::kr::dl::UniversalTag>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicStateTag<formalism::StaticTag>>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicStateTag<formalism::FluentTag>>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicStateTag<formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicGoalTag<formalism::StaticTag>>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicGoalTag<formalism::FluentTag>>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicGoalTag<formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::IntersectionTag>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::UnionTag>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::ComplementTag>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::InverseTag>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::CompositionTag>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::TransitiveClosureTag>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::RestrictionTag>>,
                                             Index<runir::kr::dl::grammar::Role<runir::kr::dl::IdentityTag>>>;
    Index<runir::kr::dl::grammar::Constructor<runir::kr::dl::RoleTag>> index;
    Variant value;
    Data() = default;
    Data(Variant value_) : value(value_) {}
    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::kr::dl::grammar::Constructor<runir::kr::dl::BooleanTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicStateTag<formalism::StaticTag>>>,
                                             Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicStateTag<formalism::FluentTag>>>,
                                             Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicStateTag<formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::NonemptyTag>>>;
    Index<runir::kr::dl::grammar::Constructor<runir::kr::dl::BooleanTag>> index;
    Variant value;
    Data() = default;
    Data(Variant value_) : value(value_) {}
    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::kr::dl::grammar::Constructor<runir::kr::dl::NumericalTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::grammar::Numerical<runir::kr::dl::CountTag>>,
                                             Index<runir::kr::dl::grammar::Numerical<runir::kr::dl::DistanceTag>>>;
    Index<runir::kr::dl::grammar::Constructor<runir::kr::dl::NumericalTag>> index;
    Variant value;
    Data() = default;
    Data(Variant value_) : value(value_) {}
    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace tyr

#endif
