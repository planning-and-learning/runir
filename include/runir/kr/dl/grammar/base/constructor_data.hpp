#ifndef RUNIR_KR_DL_GRAMMAR_BASE_CONSTRUCTOR_DATA_HPP_
#define RUNIR_KR_DL_GRAMMAR_BASE_CONSTRUCTOR_DATA_HPP_

#include "runir/kr/dl/base/declarations.hpp"
#include "runir/kr/dl/grammar/numerical_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <utility>

namespace ygg
{

template<>
struct Data<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::ConceptTag>>
{
    using Variant =
        ::cista::offset::variant<Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::BotTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::TopTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag>>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag>>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::IntersectionTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::UnionTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NegationTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::ValueRestrictionTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::ExistentialQuantificationTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtLeastNumberRestrictionTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtMostNumberRestrictionTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::ExactNumberRestrictionTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::QualifiedExactNumberRestrictionTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::RoleValueMapTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AgreementTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::RoleFillersTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::OneOfTag>>,
                                 Index<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NominalTag>>>;
    Index<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::ConceptTag>> index;
    Variant value;
    Data() = default;
    explicit Data(Variant value_) : index(), value(std::move(value_)) {}
    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(value);
    }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::RoleTag>>
{
    using Variant =
        ::cista::offset::variant<Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::UniversalTag>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag>>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag>>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::IntersectionTag>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::UnionTag>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::ComplementTag>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::InverseTag>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::CompositionTag>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::TransitiveClosureTag>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::ReflexiveTransitiveClosureTag>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::RestrictionTag>>,
                                 Index<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::IdentityTag>>>;
    Index<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::RoleTag>> index;
    Variant value;
    Data() = default;
    explicit Data(Variant value_) : index(), value(std::move(value_)) {}
    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(value);
    }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>
{
    using Variant =
        ::cista::offset::variant<Index<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag>>>,
                                 Index<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag>>>,
                                 Index<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>>>,
                                 Index<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>>>,
                                 Index<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>>>,
                                 Index<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>>,
                                 Index<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::NonemptyTag>>>;
    Index<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>> index;
    Variant value;
    Data() = default;
    explicit Data(Variant value_) : index(), value(std::move(value_)) {}
    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(value);
    }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::grammar::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::CountTag>>,
                                             Index<runir::kr::dl::grammar::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::DistanceTag>>>;
    Index<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>> index;
    Variant value;
    Data() = default;
    explicit Data(Variant value_) : index(), value(std::move(value_)) {}
    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(value);
    }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace ygg

#endif
