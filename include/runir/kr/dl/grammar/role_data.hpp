#ifndef RUNIR_GRAMMAR_ROLE_DATA_HPP_
#define RUNIR_GRAMMAR_ROLE_DATA_HPP_

#include "runir/kr/dl/grammar/concept_data.hpp"
#include "runir/kr/dl/semantics/data_helpers.hpp"

#include <concepts>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Role<Family, runir::kr::dl::UniversalTag>> :
    runir::kr::dl::semantics::NullaryData<runir::kr::dl::grammar::Role<Family, runir::kr::dl::UniversalTag>>
{
    using Base = runir::kr::dl::semantics::NullaryData<runir::kr::dl::grammar::Role<Family, runir::kr::dl::UniversalTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Role<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Role<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Role<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Role<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Role<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Role<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
             || std::same_as<Tag, runir::kr::dl::CompositionTag>)
struct Data<runir::kr::dl::grammar::Role<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Role<Family, Tag>, GrammarRoleChoice<Family>, GrammarRoleChoice<Family>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Role<Family, Tag>, GrammarRoleChoice<Family>, GrammarRoleChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::ComplementTag> || std::same_as<Tag, runir::kr::dl::InverseTag>
             || std::same_as<Tag, runir::kr::dl::TransitiveClosureTag> || std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
struct Data<runir::kr::dl::grammar::Role<Family, Tag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::grammar::Role<Family, Tag>, GrammarRoleChoice<Family>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::grammar::Role<Family, Tag>, GrammarRoleChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Role<Family, runir::kr::dl::RestrictionTag>> :
    runir::kr::dl::semantics::
        BinaryData<runir::kr::dl::grammar::Role<Family, runir::kr::dl::RestrictionTag>, GrammarRoleChoice<Family>, GrammarConceptChoice<Family>>
{
    using Base = runir::kr::dl::semantics::
        BinaryData<runir::kr::dl::grammar::Role<Family, runir::kr::dl::RestrictionTag>, GrammarRoleChoice<Family>, GrammarConceptChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Role<Family, runir::kr::dl::IdentityTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::grammar::Role<Family, runir::kr::dl::IdentityTag>, GrammarConceptChoice<Family>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::grammar::Role<Family, runir::kr::dl::IdentityTag>, GrammarConceptChoice<Family>>;
    using Base::Base;
};

}  // namespace ygg

#endif
