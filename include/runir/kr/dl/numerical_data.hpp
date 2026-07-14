#ifndef RUNIR_KR_DL_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/boolean_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
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
        ygg::clear(index);
        ygg::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Numerical<Family, runir::kr::dl::DistanceTag>> :
    runir::kr::dl::semantics::TernaryData<runir::kr::dl::Numerical<Family, runir::kr::dl::DistanceTag>, DlConcept<Family>, DlRole<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::semantics::
        TernaryData<runir::kr::dl::Numerical<Family, runir::kr::dl::DistanceTag>, DlConcept<Family>, DlRole<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> :
    runir::kr::dl::semantics::ReferenceData<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                            runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>
{
    using Base =
        runir::kr::dl::semantics::ReferenceData<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                                runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Numerical<Family, runir::kr::dl::NumericalConstantTag>> :
    runir::kr::dl::semantics::IdentifierData<runir::kr::dl::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>
{
    using Base = runir::kr::dl::semantics::IdentifierData<runir::kr::dl::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>;
    using Base::Base;
};

// Binary numerical operators (add/sub/mul/div/min/max): a Numerical over two Numerical operands.
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalBinaryTag Tag>
struct Data<runir::kr::dl::Numerical<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Numerical<Family, Tag>,
                                         runir::kr::dl::Constructor<Family, runir::kr::dl::NumericalTag>,
                                         runir::kr::dl::Constructor<Family, runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Numerical<Family, Tag>,
                                                      runir::kr::dl::Constructor<Family, runir::kr::dl::NumericalTag>,
                                                      runir::kr::dl::Constructor<Family, runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
