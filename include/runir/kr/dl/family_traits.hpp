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

using ExtConceptConstructorTags = ygg::ConcatTypeListsT<BaseConceptConstructorTags, ygg::TypeList<RegisterTag, ArgumentTag<ConceptTag>>>;
using ExtRoleConstructorTags = ygg::ConcatTypeListsT<BaseRoleConstructorTags, ygg::TypeList<RegisterTag, ArgumentTag<RoleTag>>>;
using NumericalBinaryConstructorTags = ygg::TypeList<AddTag, SubTag, MulTag, DivTag, MinTag, MaxTag>;
using ArithmeticConstructorTags = ygg::ConcatTypeListsT<ygg::TypeList<NumericalConstantTag>, NumericalBinaryConstructorTags>;
using ExtNumericalConstructorTags = ygg::ConcatTypeListsT<BaseNumericalConstructorTags, ArithmeticConstructorTags, ygg::TypeList<ArgumentTag<NumericalTag>>>;

using UnsComparisonConstructorTags = ygg::TypeList<EqTag<BooleanTag>,
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

using UnsLogicalConstructorTags = ygg::TypeList<AndTag, OrTag, NotTag>;

using UnsConceptConstructorTags = BaseConceptConstructorTags;
using UnsRoleConstructorTags = BaseRoleConstructorTags;
using UnsBooleanConstructorTags = ygg::ConcatTypeListsT<
    BaseBooleanConstructorTags,
    ygg::ConcatTypeListsT<UnsComparisonConstructorTags, ygg::ConcatTypeListsT<ygg::TypeList<BooleanConstantTag>, UnsLogicalConstructorTags>>>;
using UnsNumericalConstructorTags = ygg::ConcatTypeListsT<BaseNumericalConstructorTags, ArithmeticConstructorTags>;

/// Feature generation grammars and their CNF build only these constructors.
using GenerationConceptConstructorTags = BaseConceptConstructorTags;
using GenerationRoleConstructorTags = BaseRoleConstructorTags;
using GenerationBooleanConstructorTags = BaseBooleanConstructorTags;
using GenerationNumericalConstructorTags = BaseNumericalConstructorTags;

/// Ext programs use every constructor: the Uns comparisons, constants and connectives too.
using ExtBooleanConstructorTags = ygg::ConcatTypeListsT<UnsBooleanConstructorTags, ygg::TypeList<ArgumentTag<BooleanTag>>>;

}  // namespace runir::kr::dl

#endif
