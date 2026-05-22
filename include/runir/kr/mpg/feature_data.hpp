#ifndef RUNIR_KR_MPG_FEATURE_DATA_HPP_
#define RUNIR_KR_MPG_FEATURE_DATA_HPP_

#include "runir/kr/mpg/dl/declarations.hpp"
#include "runir/kr/mpg/dl/feature_index.hpp"
#include "runir/kr/mpg/feature_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::mpg::Feature<runir::kr::mpg::dl::ConceptFeature>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::mpg::ConcreteFeature<runir::kr::DlTag, runir::kr::mpg::dl::ConceptFeature>>>;

    Index<runir::kr::mpg::Feature<runir::kr::mpg::dl::ConceptFeature>> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(std::move(value_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::kr::mpg::Feature<runir::kr::mpg::dl::BooleanFeature>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::mpg::ConcreteFeature<runir::kr::DlTag, runir::kr::mpg::dl::BooleanFeature>>>;

    Index<runir::kr::mpg::Feature<runir::kr::mpg::dl::BooleanFeature>> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(std::move(value_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::kr::mpg::Feature<runir::kr::mpg::dl::NumericalFeature>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::mpg::ConcreteFeature<runir::kr::DlTag, runir::kr::mpg::dl::NumericalFeature>>>;

    Index<runir::kr::mpg::Feature<runir::kr::mpg::dl::NumericalFeature>> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(std::move(value_)) {}

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
