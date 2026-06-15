#ifndef RUNIR_KR_DL_CNF_GRAMMAR_UNS_CONSTRUCTOR_DATA_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_UNS_CONSTRUCTOR_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/concept_data.hpp"
#include "runir/kr/dl/cnf_grammar/numerical_data.hpp"
#include "runir/kr/dl/cnf_grammar/role_data.hpp"
#include "runir/kr/dl/cnf_grammar/uns/boolean_data.hpp"
#include "runir/kr/dl/uns/declarations.hpp"
#include "runir/kr/dl/cnf_grammar/uns/numerical_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <utility>

namespace ygg
{

template<>
struct Data<runir::kr::dl::cnf_grammar::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::BotTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::TopTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::IntersectionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::UnionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::NegationTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::ValueRestrictionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::ExistentialQuantificationTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtLeastNumberRestrictionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtMostNumberRestrictionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::ExactNumberRestrictionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::QualifiedExactNumberRestrictionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::RoleValueMapTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AgreementTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::RoleFillersTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::OneOfTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::NominalTag>>>;

    Index<runir::kr::dl::cnf_grammar::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>> index;
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
struct Data<runir::kr::dl::cnf_grammar::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::UniversalTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::IntersectionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::UnionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::ComplementTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::InverseTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::CompositionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::TransitiveClosureTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::ReflexiveTransitiveClosureTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::RestrictionTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Role<runir::kr::UnsFamilyTag, runir::kr::dl::IdentityTag>>>;

    Index<runir::kr::dl::cnf_grammar::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>> index;
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
struct Data<runir::kr::dl::cnf_grammar::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NonemptyTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::EqTag<runir::kr::dl::BooleanTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NeqTag<runir::kr::dl::BooleanTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::LtTag<runir::kr::dl::BooleanTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::LeTag<runir::kr::dl::BooleanTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::GtTag<runir::kr::dl::BooleanTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::GeTag<runir::kr::dl::BooleanTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::EqTag<runir::kr::dl::NumericalTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NeqTag<runir::kr::dl::NumericalTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::LtTag<runir::kr::dl::NumericalTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::LeTag<runir::kr::dl::NumericalTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::GtTag<runir::kr::dl::NumericalTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::GeTag<runir::kr::dl::NumericalTag>>>,
                                             Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanConstantTag>>>;

    Index<runir::kr::dl::cnf_grammar::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>> index;
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
struct Data<runir::kr::dl::cnf_grammar::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::cnf_grammar::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::CountTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::DistanceTag>>,
                                             Index<runir::kr::dl::cnf_grammar::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalConstantTag>>>;

    Index<runir::kr::dl::cnf_grammar::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>> index;
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
