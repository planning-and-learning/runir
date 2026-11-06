#ifndef RUNIR_KR_DL_CONCEPT_DATA_HPP_
#define RUNIR_KR_DL_CONCEPT_DATA_HPP_

#include "runir/kr/dl/data_helpers.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <concepts>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>

namespace runir::kr::dl
{

using DlConcept = runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>;

using DlRole = runir::kr::dl::Constructor<runir::kr::dl::RoleTag>;

/// The arguments of count and nonempty: a concept, a role, or a query.
using DlArgumentTypes = boost::mp11::mp_list<DlConcept, DlRole, runir::kr::dl::Query<>>;

/// The vertex (sources/targets) and edge arguments of distance: a concept or role, or a query.
using VertexArgumentTypes = boost::mp11::mp_list<DlConcept, runir::kr::dl::Query<>>;
using EdgeArgumentTypes = boost::mp11::mp_list<DlRole, runir::kr::dl::Query<>>;

}  // namespace runir::kr::dl

namespace urd
{

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::BotTag>> : runir::kr::dl::NullaryData<runir::kr::dl::Concept<runir::kr::dl::BotTag>>
{
    using Base = runir::kr::dl::NullaryData<runir::kr::dl::Concept<runir::kr::dl::BotTag>>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::TopTag>> : runir::kr::dl::NullaryData<runir::kr::dl::Concept<runir::kr::dl::TopTag>>
{
    using Base = runir::kr::dl::NullaryData<runir::kr::dl::Concept<runir::kr::dl::TopTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::Concept<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::Concept<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::Concept<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::Concept<runir::kr::dl::AtomicGoalTag<T>>> : runir::kr::dl::PredicateData<runir::kr::dl::Concept<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::Concept<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::IntersectionTag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Concept<runir::kr::dl::IntersectionTag>, runir::kr::dl::DlConcept, runir::kr::dl::DlConcept>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<runir::kr::dl::IntersectionTag>, runir::kr::dl::DlConcept, runir::kr::dl::DlConcept>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::UnionTag>> : runir::kr::dl::BinaryData<runir::kr::dl::Concept<runir::kr::dl::UnionTag>, runir::kr::dl::DlConcept, runir::kr::dl::DlConcept>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<runir::kr::dl::UnionTag>, runir::kr::dl::DlConcept, runir::kr::dl::DlConcept>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::NegationTag>> : runir::kr::dl::UnaryData<runir::kr::dl::Concept<runir::kr::dl::NegationTag>, runir::kr::dl::DlConcept>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::Concept<runir::kr::dl::NegationTag>, runir::kr::dl::DlConcept>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::ValueRestrictionTag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Concept<runir::kr::dl::ValueRestrictionTag>, runir::kr::dl::DlRole, runir::kr::dl::DlConcept>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<runir::kr::dl::ValueRestrictionTag>, runir::kr::dl::DlRole, runir::kr::dl::DlConcept>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::ExistentialQuantificationTag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Concept<runir::kr::dl::ExistentialQuantificationTag>, runir::kr::dl::DlRole, runir::kr::dl::DlConcept>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<runir::kr::dl::ExistentialQuantificationTag>, runir::kr::dl::DlRole, runir::kr::dl::DlConcept>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::AtLeastNumberRestrictionTag>> :
    runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<runir::kr::dl::AtLeastNumberRestrictionTag>, runir::kr::dl::DlRole>
{
    using Base = runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<runir::kr::dl::AtLeastNumberRestrictionTag>, runir::kr::dl::DlRole>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::AtMostNumberRestrictionTag>> :
    runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<runir::kr::dl::AtMostNumberRestrictionTag>, runir::kr::dl::DlRole>
{
    using Base = runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<runir::kr::dl::AtMostNumberRestrictionTag>, runir::kr::dl::DlRole>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::ExactNumberRestrictionTag>> :
    runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<runir::kr::dl::ExactNumberRestrictionTag>, runir::kr::dl::DlRole>
{
    using Base = runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<runir::kr::dl::ExactNumberRestrictionTag>, runir::kr::dl::DlRole>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
struct data<runir::kr::dl::Concept<Tag>> : runir::kr::dl::QualifiedNumberRestrictionData<runir::kr::dl::Concept<Tag>, runir::kr::dl::DlRole, runir::kr::dl::DlConcept>
{
    using Base = runir::kr::dl::QualifiedNumberRestrictionData<runir::kr::dl::Concept<Tag>, runir::kr::dl::DlRole, runir::kr::dl::DlConcept>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
struct data<runir::kr::dl::Concept<Tag>> : runir::kr::dl::BinaryData<runir::kr::dl::Concept<Tag>, runir::kr::dl::DlRole, runir::kr::dl::DlRole>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<Tag>, runir::kr::dl::DlRole, runir::kr::dl::DlRole>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::RoleFillersTag>> :
    runir::kr::dl::RoleFillersData<runir::kr::dl::Concept<runir::kr::dl::RoleFillersTag>, runir::kr::dl::DlRole>
{
    using Base = runir::kr::dl::RoleFillersData<runir::kr::dl::Concept<runir::kr::dl::RoleFillersTag>, runir::kr::dl::DlRole>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::OneOfTag>> : runir::kr::dl::ObjectListData<runir::kr::dl::Concept<runir::kr::dl::OneOfTag>>
{
    using Base = runir::kr::dl::ObjectListData<runir::kr::dl::Concept<runir::kr::dl::OneOfTag>>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::NominalTag>> : runir::kr::dl::ObjectData<runir::kr::dl::Concept<runir::kr::dl::NominalTag>>
{
    using Base = runir::kr::dl::ObjectData<runir::kr::dl::Concept<runir::kr::dl::NominalTag>>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::RegisterTag>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Concept<runir::kr::dl::RegisterTag>, runir::kr::dl::Register<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Concept<runir::kr::dl::RegisterTag>, runir::kr::dl::Register<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Concept<runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Concept<runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                 runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Concept<runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                              runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace urd

#endif
