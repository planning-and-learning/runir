#ifndef RUNIR_KR_PS_EXT_DL_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_EXT_DL_FEATURE_DATA_HPP_

#include "runir/kr/dl/constructor_index.hpp"
#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/dl/feature_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <utility>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::dl::ConceptTag>>
{
    Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::dl::ConceptTag>> index;
    Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>> feature;
    ::cista::offset::string symbol;

    Data() = default;
    Data(Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>> feature_,
         ::cista::offset::string symbol_) :
        index(),
        feature(feature_),
        symbol(std::move(symbol_))
    {
    }
    Data(Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>> feature_,
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
struct Data<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::dl::RoleTag>>
{
    Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::dl::RoleTag>> index;
    Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>> feature;
    ::cista::offset::string symbol;

    Data() = default;
    Data(Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>> feature_, ::cista::offset::string symbol_) :
        index(),
        feature(feature_),
        symbol(std::move(symbol_))
    {
    }
    Data(Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>> feature_, const std::string& symbol_) :
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
struct Data<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>
{
    Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>> index;
    Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>> feature;
    ::cista::offset::string symbol;

    Data() = default;
    Data(Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>> feature_,
         ::cista::offset::string symbol_) :
        index(),
        feature(feature_),
        symbol(std::move(symbol_))
    {
    }
    Data(Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>> feature_,
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
struct Data<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>
{
    Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>> index;
    Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>> feature;
    ::cista::offset::string symbol;

    Data() = default;
    Data(Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>> feature_,
         ::cista::offset::string symbol_) :
        index(),
        feature(feature_),
        symbol(std::move(symbol_))
    {
    }
    Data(Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>> feature_,
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
