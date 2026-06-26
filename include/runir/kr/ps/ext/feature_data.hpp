#ifndef RUNIR_KR_PS_EXT_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_EXT_FEATURE_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/dl/feature_index.hpp"
#include "runir/kr/ps/ext/feature_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::Feature<runir::kr::dl::ConceptTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::dl::ConceptTag>>>;

    Index<runir::kr::ps::ext::Feature<runir::kr::dl::ConceptTag>> index;
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
struct Data<runir::kr::ps::ext::Feature<runir::kr::dl::RoleTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::dl::RoleTag>>>;

    Index<runir::kr::ps::ext::Feature<runir::kr::dl::RoleTag>> index;
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
struct Data<runir::kr::ps::ext::Feature<runir::kr::ps::dl::BooleanFeature>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>>;

    Index<runir::kr::ps::ext::Feature<runir::kr::ps::dl::BooleanFeature>> index;
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
struct Data<runir::kr::ps::ext::Feature<runir::kr::ps::dl::NumericalFeature>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>>;

    Index<runir::kr::ps::ext::Feature<runir::kr::ps::dl::NumericalFeature>> index;
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

}  // namespace ygg

#endif
