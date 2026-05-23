#ifndef RUNIR_KR_DL_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/boolean_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <utility>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Numerical<Family, runir::kr::dl::CountTag>>
{
    using ConstructorVariant = ::cista::offset::variant<Index<DlConcept<Family>>, Index<DlRole<Family>>>;

    Index<runir::kr::dl::Numerical<Family, runir::kr::dl::CountTag>> index;
    ConstructorVariant arg;

    Data() = default;
    explicit Data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Numerical<Family, runir::kr::dl::DistanceTag>> :
    runir::kr::dl::semantics::
        TernaryData<runir::kr::dl::Numerical<Family, runir::kr::dl::DistanceTag>, DlConcept<Family>, DlRole<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::semantics::
        TernaryData<runir::kr::dl::Numerical<Family, runir::kr::dl::DistanceTag>, DlConcept<Family>, DlRole<Family>, DlConcept<Family>>;
    using Base::Base;
};

}  // namespace tyr

#endif
