#ifndef RUNIR_KR_DL_CONCEPT_DATA_HPP_
#define RUNIR_KR_DL_CONCEPT_DATA_HPP_

#include "runir/kr/dl/data_helpers.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <concepts>
#include <urd/elements/flat.hpp>

namespace urd
{

template<typename F, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::BotTag> || std::same_as<Tag, runir::kr::dl::TopTag>)
struct data<runir::kr::dl::Concept<F, Tag>> : runir::kr::dl::NullaryData<runir::kr::dl::Concept<F, Tag>>
{
    using Base = runir::kr::dl::NullaryData<runir::kr::dl::Concept<F, Tag>>;
    using Base::Base;
};

template<typename F, typename Tag>
    requires runir::kr::dl::is_atomic_state_tag_v<Tag>
struct data<runir::kr::dl::Concept<F, Tag>> : runir::kr::dl::AtomData<runir::kr::dl::Concept<F, Tag>, runir::kr::dl::TagFactKind<Tag>>
{
    using Base = runir::kr::dl::AtomData<runir::kr::dl::Concept<F, Tag>, runir::kr::dl::TagFactKind<Tag>>;
    using Base::Base;
};

template<typename F, typename Tag>
    requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
struct data<runir::kr::dl::Concept<F, Tag>> : runir::kr::dl::PredicateData<runir::kr::dl::Concept<F, Tag>, runir::kr::dl::TagFactKind<Tag>>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::Concept<F, Tag>, runir::kr::dl::TagFactKind<Tag>>;
    using Base::Base;
};

template<typename F, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>)
struct data<runir::kr::dl::Concept<F, Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Concept<F, Tag>,
                              runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>,
                              runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<F, Tag>,
                                           runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>,
                                           runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::NegationTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::Concept<F, runir::kr::dl::NegationTag>, runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::Concept<F, runir::kr::dl::NegationTag>, runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<typename F, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
struct data<runir::kr::dl::Concept<F, Tag>> :
    runir::kr::dl::
        BinaryData<runir::kr::dl::Concept<F, Tag>, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::
        BinaryData<runir::kr::dl::Concept<F, Tag>, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<typename F, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
struct data<runir::kr::dl::Concept<F, Tag>> :
    runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<F, Tag>, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<F, Tag>, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<typename F, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
struct data<runir::kr::dl::Concept<F, Tag>> :
    runir::kr::dl::QualifiedNumberRestrictionData<runir::kr::dl::Concept<F, Tag>,
                                                  runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>,
                                                  runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::QualifiedNumberRestrictionData<runir::kr::dl::Concept<F, Tag>,
                                                               runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>,
                                                               runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<typename F, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
struct data<runir::kr::dl::Concept<F, Tag>> :
    runir::kr::dl::
        BinaryData<runir::kr::dl::Concept<F, Tag>, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::
        BinaryData<runir::kr::dl::Concept<F, Tag>, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::RoleFillersTag>> :
    runir::kr::dl::RoleFillersData<runir::kr::dl::Concept<F, runir::kr::dl::RoleFillersTag>, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::RoleFillersData<runir::kr::dl::Concept<F, runir::kr::dl::RoleFillersTag>, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::OneOfTag>> : runir::kr::dl::ObjectListData<runir::kr::dl::Concept<F, runir::kr::dl::OneOfTag>>
{
    using Base = runir::kr::dl::ObjectListData<runir::kr::dl::Concept<F, runir::kr::dl::OneOfTag>>;
    using Base::Base;
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::NominalTag>> : runir::kr::dl::ObjectData<runir::kr::dl::Concept<F, runir::kr::dl::NominalTag>>
{
    using Base = runir::kr::dl::ObjectData<runir::kr::dl::Concept<F, runir::kr::dl::NominalTag>>;
    using Base::Base;
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::RegisterTag>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Concept<F, runir::kr::dl::RegisterTag>, runir::kr::dl::Register<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Concept<F, runir::kr::dl::RegisterTag>, runir::kr::dl::Register<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Concept<F, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                 runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Concept<F, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                              runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace urd

#endif
