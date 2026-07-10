#ifndef RUNIR_KR_PS_BASE_DL_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_BASE_DL_FEATURE_DATA_HPP_

#include "runir/kr/dl/constructor_index.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/feature_index.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
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
        ygg::clear(index);
        ygg::clear(value);
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
        ygg::clear(index);
        ygg::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>
{
    Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>> index;
    Index<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>> feature;
    ::cista::offset::string symbol;

    Data() = default;
    Data(Index<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>> feature_,
         ::cista::offset::string symbol_) :
        index(),
        feature(feature_),
        symbol(std::move(symbol_))
    {
    }
    Data(Index<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>> feature_,
         const std::string& symbol_) :
        index(),
        feature(feature_),
        symbol(symbol_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(feature);
        ygg::clear(symbol);
    }

    auto cista_members() const noexcept { return std::tie(index, feature, symbol); }
    auto identifying_members() const noexcept { return std::tie(feature, symbol); }
};

template<>
struct Data<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>
{
    Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>> index;
    Index<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>> feature;
    ::cista::offset::string symbol;

    Data() = default;
    Data(Index<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>> feature_,
         ::cista::offset::string symbol_) :
        index(),
        feature(feature_),
        symbol(std::move(symbol_))
    {
    }
    Data(Index<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>> feature_,
         const std::string& symbol_) :
        index(),
        feature(feature_),
        symbol(symbol_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(feature);
        ygg::clear(symbol);
    }

    auto cista_members() const noexcept { return std::tie(index, feature, symbol); }
    auto identifying_members() const noexcept { return std::tie(feature, symbol); }
};

}  // namespace ygg

#endif
