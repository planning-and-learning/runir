#ifndef RUNIR_KR_DL_BOOLEAN_DATA_HPP_
#define RUNIR_KR_DL_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/role_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <utility>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Boolean<Family, runir::kr::dl::NonemptyTag>>
{
    using ConstructorVariant = ::cista::offset::variant<Index<DlConcept<Family>>, Index<DlRole<Family>>>;

    Index<runir::kr::dl::Boolean<Family, runir::kr::dl::NonemptyTag>> index;
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

}  // namespace tyr

#endif
