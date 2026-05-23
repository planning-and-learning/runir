#ifndef RUNIR_KR_DL_EXT_CONSTRUCTOR_DATA_HPP_
#define RUNIR_KR_DL_EXT_CONSTRUCTOR_DATA_HPP_

#include "runir/kr/dl/ext/concept_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <utility>

namespace tyr
{

template<>
struct Data<runir::kr::dl::Constructor<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ConceptTag>>
{
    using Family = runir::kr::dl::ExtFamilyTag;
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::Concept<Family, runir::kr::dl::BotTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::TopTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag>>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag>>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::IntersectionTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::UnionTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::NegationTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::ValueRestrictionTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::QualifiedExactNumberRestrictionTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::RoleValueMapTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::AgreementTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::RoleFillersTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::OneOfTag>>,
                                             Index<runir::kr::dl::Concept<Family, runir::kr::dl::NominalTag>>,
                                             Index<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterTag>>>;

    Index<runir::kr::dl::Constructor<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ConceptTag>> index;
    Variant value;

    Data() = default;
    explicit Data(Variant value_) : index(), value(std::move(value_)) {}

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
