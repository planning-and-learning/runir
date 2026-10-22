// declarations.hpp includes this header in its middle; including it first, before the guard, lets this header
// also compile on its own.
#include "runir/kr/dl/declarations.hpp"

#ifndef RUNIR_KR_DL_FAMILY_TRAITS_HPP_
#define RUNIR_KR_DL_FAMILY_TRAITS_HPP_

namespace runir::kr::dl
{

using BaseConceptConstructorTags = ygg::TypeList<BotTag,
                                                 TopTag,
                                                 AtomicStateTag<tyr::formalism::StaticTag>,
                                                 AtomicStateTag<tyr::formalism::FluentTag>,
                                                 AtomicStateTag<tyr::formalism::DerivedTag>,
                                                 AtomicGoalTag<tyr::formalism::StaticTag>,
                                                 AtomicGoalTag<tyr::formalism::FluentTag>,
                                                 AtomicGoalTag<tyr::formalism::DerivedTag>,
                                                 IntersectionTag,
                                                 UnionTag,
                                                 NegationTag,
                                                 ValueRestrictionTag,
                                                 ExistentialQuantificationTag,
                                                 AtLeastNumberRestrictionTag,
                                                 AtMostNumberRestrictionTag,
                                                 ExactNumberRestrictionTag,
                                                 QualifiedAtLeastNumberRestrictionTag,
                                                 QualifiedAtMostNumberRestrictionTag,
                                                 QualifiedExactNumberRestrictionTag,
                                                 RoleValueMapTag,
                                                 AgreementTag,
                                                 RoleFillersTag,
                                                 OneOfTag,
                                                 NominalTag>;

using BaseRoleConstructorTags = ygg::TypeList<UniversalTag,
                                              AtomicStateTag<tyr::formalism::StaticTag>,
                                              AtomicStateTag<tyr::formalism::FluentTag>,
                                              AtomicStateTag<tyr::formalism::DerivedTag>,
                                              AtomicGoalTag<tyr::formalism::StaticTag>,
                                              AtomicGoalTag<tyr::formalism::FluentTag>,
                                              AtomicGoalTag<tyr::formalism::DerivedTag>,
                                              IntersectionTag,
                                              UnionTag,
                                              ComplementTag,
                                              InverseTag,
                                              CompositionTag,
                                              TransitiveClosureTag,
                                              ReflexiveTransitiveClosureTag,
                                              RestrictionTag,
                                              IdentityTag>;

using BaseBooleanConstructorTags = ygg::TypeList<AtomicStateTag<tyr::formalism::StaticTag>,
                                                 AtomicStateTag<tyr::formalism::FluentTag>,
                                                 AtomicStateTag<tyr::formalism::DerivedTag>,
                                                 AtomicGoalTag<tyr::formalism::StaticTag>,
                                                 AtomicGoalTag<tyr::formalism::FluentTag>,
                                                 AtomicGoalTag<tyr::formalism::DerivedTag>,
                                                 NonemptyTag>;

using BaseNumericalConstructorTags = ygg::TypeList<CountTag, DistanceTag>;

using NumericalBinaryConstructorTags = ygg::TypeList<AddTag, SubTag, MulTag, DivTag, MinTag, MaxTag>;
using ArithmeticConstructorTags = ygg::ConcatTypeListsT<ygg::TypeList<NumericalConstantTag>, NumericalBinaryConstructorTags>;

using ComparisonConstructorTags = ygg::TypeList<EqTag<BooleanTag>,
                                                NeqTag<BooleanTag>,
                                                LtTag<BooleanTag>,
                                                LeTag<BooleanTag>,
                                                GtTag<BooleanTag>,
                                                GeTag<BooleanTag>,
                                                EqTag<NumericalTag>,
                                                NeqTag<NumericalTag>,
                                                LtTag<NumericalTag>,
                                                LeTag<NumericalTag>,
                                                GtTag<NumericalTag>,
                                                GeTag<NumericalTag>>;

using LogicalConstructorTags = ygg::TypeList<AndTag, OrTag, NotTag>;

/// Every language shares one constructor set; a language's parser rejects what it does not support.
using ConceptConstructorTags = ygg::ConcatTypeListsT<BaseConceptConstructorTags, ygg::TypeList<RegisterTag, ArgumentTag<ConceptTag>>>;
using RoleConstructorTags = ygg::ConcatTypeListsT<BaseRoleConstructorTags, ygg::TypeList<RegisterTag, ArgumentTag<RoleTag>>>;
using BooleanConstructorTags = ygg::ConcatTypeListsT<BaseBooleanConstructorTags,
                                                     ComparisonConstructorTags,
                                                     ygg::TypeList<BooleanConstantTag>,
                                                     LogicalConstructorTags,
                                                     ygg::TypeList<ArgumentTag<BooleanTag>>>;
using NumericalConstructorTags = ygg::ConcatTypeListsT<BaseNumericalConstructorTags, ArithmeticConstructorTags, ygg::TypeList<ArgumentTag<NumericalTag>>>;

/// Feature generation grammars and their CNF build only these constructors.
using GenerationConceptConstructorTags = BaseConceptConstructorTags;
using GenerationRoleConstructorTags = BaseRoleConstructorTags;
using GenerationBooleanConstructorTags = BaseBooleanConstructorTags;
using GenerationNumericalConstructorTags = BaseNumericalConstructorTags;

}  // namespace runir::kr::dl

#endif
