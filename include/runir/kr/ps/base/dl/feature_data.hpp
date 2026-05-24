#ifndef RUNIR_KR_PS_BASE_DL_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_BASE_DL_FEATURE_DATA_HPP_

#include "runir/kr/dl/constructor_index.hpp"
#include "runir/kr/ps/base/dl/feature_index.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>
{
    using Variant =
        ::cista::offset::variant<Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>>;

    Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>> index;
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
struct Data<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>
{
    using Variant =
        ::cista::offset::variant<Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>>;

    Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>> index;
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
struct Data<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>
{
    Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>> index;
    Index<runir::kr::dl::Constructor<runir::kr::dl::BaseFamilyTag, runir::kr::dl::BooleanTag>> feature;
    ::cista::offset::string symbol;
    ::cista::offset::string description;

    Data() = default;
    Data(Index<runir::kr::dl::Constructor<runir::kr::dl::BaseFamilyTag, runir::kr::dl::BooleanTag>> feature_,
         ::cista::offset::string symbol_,
         ::cista::offset::string description_) :
        index(),
        feature(feature_),
        symbol(std::move(symbol_)),
        description(std::move(description_))
    {
    }
    Data(Index<runir::kr::dl::Constructor<runir::kr::dl::BaseFamilyTag, runir::kr::dl::BooleanTag>> feature_,
         const std::string& symbol_,
         const std::string& description_) :
        index(),
        feature(feature_),
        symbol(symbol_),
        description(description_)
    {
    }

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(feature);
        tyr::clear(symbol);
        tyr::clear(description);
    }

    auto cista_members() const noexcept { return std::tie(index, feature, symbol, description); }
    auto identifying_members() const noexcept { return std::tie(feature, symbol, description); }
};

template<>
struct Data<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>
{
    Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>> index;
    Index<runir::kr::dl::Constructor<runir::kr::dl::BaseFamilyTag, runir::kr::dl::NumericalTag>> feature;
    ::cista::offset::string symbol;
    ::cista::offset::string description;

    Data() = default;
    Data(Index<runir::kr::dl::Constructor<runir::kr::dl::BaseFamilyTag, runir::kr::dl::NumericalTag>> feature_,
         ::cista::offset::string symbol_,
         ::cista::offset::string description_) :
        index(),
        feature(feature_),
        symbol(std::move(symbol_)),
        description(std::move(description_))
    {
    }
    Data(Index<runir::kr::dl::Constructor<runir::kr::dl::BaseFamilyTag, runir::kr::dl::NumericalTag>> feature_,
         const std::string& symbol_,
         const std::string& description_) :
        index(),
        feature(feature_),
        symbol(symbol_),
        description(description_)
    {
    }

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(feature);
        tyr::clear(symbol);
        tyr::clear(description);
    }

    auto cista_members() const noexcept { return std::tie(index, feature, symbol, description); }
    auto identifying_members() const noexcept { return std::tie(feature, symbol, description); }
};

}  // namespace tyr

#endif
