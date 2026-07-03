#ifndef RUNIR_KR_DL_FAMILY_TRAITS_HPP_
#define RUNIR_KR_DL_FAMILY_TRAITS_HPP_

#include "runir/kr/dl/declarations.hpp"

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
using ExtBooleanConstructorTags = ygg::ConcatTypeListsT<BaseBooleanConstructorTags, ygg::TypeList<ArgumentTag<BooleanTag>>>;
using ExtNumericalConstructorTags = ygg::ConcatTypeListsT<BaseNumericalConstructorTags, ygg::TypeList<ArgumentTag<NumericalTag>>>;

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
using UnsNumericalBinaryConstructorTags = ygg::TypeList<AddTag, SubTag, MulTag, DivTag, MinTag, MaxTag>;

using UnsConceptConstructorTags = BaseConceptConstructorTags;
using UnsRoleConstructorTags = BaseRoleConstructorTags;
using UnsBooleanConstructorTags = ygg::ConcatTypeListsT<
    BaseBooleanConstructorTags,
    ygg::ConcatTypeListsT<UnsComparisonConstructorTags, ygg::ConcatTypeListsT<ygg::TypeList<BooleanConstantTag>, UnsLogicalConstructorTags>>>;
using UnsNumericalConstructorTags =
    ygg::ConcatTypeListsT<BaseNumericalConstructorTags, ygg::ConcatTypeListsT<ygg::TypeList<NumericalConstantTag>, UnsNumericalBinaryConstructorTags>>;

template<>
struct DlFamilyTraits<runir::kr::BaseFamilyTag>
{
    using ConceptConstructorTags = BaseConceptConstructorTags;
    using RoleConstructorTags = BaseRoleConstructorTags;
    using BooleanConstructorTags = BaseBooleanConstructorTags;
    using NumericalConstructorTags = BaseNumericalConstructorTags;
};

template<>
struct DlFamilyTraits<runir::kr::ExtFamilyTag>
{
    using ConceptConstructorTags = ExtConceptConstructorTags;
    using RoleConstructorTags = ExtRoleConstructorTags;
    using BooleanConstructorTags = ExtBooleanConstructorTags;
    using NumericalConstructorTags = ExtNumericalConstructorTags;
};

template<>
struct DlFamilyTraits<runir::kr::UnsFamilyTag>
{
    using ConceptConstructorTags = UnsConceptConstructorTags;
    using RoleConstructorTags = UnsRoleConstructorTags;
    using BooleanConstructorTags = UnsBooleanConstructorTags;
    using NumericalConstructorTags = UnsNumericalConstructorTags;
};

}  // namespace runir::kr::dl

#endif
