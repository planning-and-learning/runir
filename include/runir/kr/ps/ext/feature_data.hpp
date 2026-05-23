#ifndef RUNIR_KR_PS_EXT_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_EXT_FEATURE_DATA_HPP_

#include "runir/kr/ps/ext/dl/declarations.hpp"
#include "runir/kr/ps/ext/dl/feature_index.hpp"
#include "runir/kr/ps/ext/feature_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::ps::ext::Feature<runir::kr::ps::ext::dl::ConceptFeature>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::ps::ext::dl::ConceptFeature>>>;

    Index<runir::kr::ps::ext::Feature<runir::kr::ps::ext::dl::ConceptFeature>> index;
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
struct Data<runir::kr::ps::ext::Feature<runir::kr::ps::ext::dl::BooleanFeature>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::ps::ext::dl::BooleanFeature>>>;

    Index<runir::kr::ps::ext::Feature<runir::kr::ps::ext::dl::BooleanFeature>> index;
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
struct Data<runir::kr::ps::ext::Feature<runir::kr::ps::ext::dl::NumericalFeature>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::ps::ext::dl::NumericalFeature>>>;

    Index<runir::kr::ps::ext::Feature<runir::kr::ps::ext::dl::NumericalFeature>> index;
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
