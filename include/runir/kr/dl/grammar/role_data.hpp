#ifndef RUNIR_GRAMMAR_ROLE_DATA_HPP_
#define RUNIR_GRAMMAR_ROLE_DATA_HPP_

#include "runir/kr/dl/data_helpers.hpp"
#include "runir/kr/dl/grammar/concept_data.hpp"

#include <concepts>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::dl::grammar::Role<runir::kr::dl::UniversalTag>> : runir::kr::dl::NullaryData<runir::kr::dl::grammar::Role<runir::kr::dl::UniversalTag>>
{
    using Base = runir::kr::dl::NullaryData<runir::kr::dl::grammar::Role<runir::kr::dl::UniversalTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
             || std::same_as<Tag, runir::kr::dl::CompositionTag>)
struct data<runir::kr::dl::grammar::Role<Tag>> : runir::kr::dl::BinaryData<runir::kr::dl::grammar::Role<Tag>, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarRoleChoice>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Role<Tag>, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarRoleChoice>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::ComplementTag> || std::same_as<Tag, runir::kr::dl::InverseTag>
             || std::same_as<Tag, runir::kr::dl::TransitiveClosureTag> || std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
struct data<runir::kr::dl::grammar::Role<Tag>> : runir::kr::dl::UnaryData<runir::kr::dl::grammar::Role<Tag>, runir::kr::dl::grammar::GrammarRoleChoice>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::grammar::Role<Tag>, runir::kr::dl::grammar::GrammarRoleChoice>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Role<runir::kr::dl::RestrictionTag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::grammar::Role<runir::kr::dl::RestrictionTag>, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarConceptChoice>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Role<runir::kr::dl::RestrictionTag>, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarConceptChoice>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Role<runir::kr::dl::IdentityTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::IdentityTag>, runir::kr::dl::grammar::GrammarConceptChoice>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::IdentityTag>, runir::kr::dl::grammar::GrammarConceptChoice>;
    using Base::Base;
};

}  // namespace urd

#endif
