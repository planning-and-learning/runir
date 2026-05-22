#ifndef RUNIR_KR_MPG_DL_CONSTRUCTOR_DATA_HPP_
#define RUNIR_KR_MPG_DL_CONSTRUCTOR_DATA_HPP_

#include "runir/kr/dl/constructor_index.hpp"
#include "runir/kr/mpg/dl/concept_index.hpp"
#include "runir/kr/mpg/dl/constructor_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <utility>

namespace tyr
{

template<>
struct Data<runir::kr::mpg::dl::Constructor<runir::kr::dl::ConceptTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>,
                                             Index<runir::kr::mpg::dl::Concept<runir::kr::mpg::dl::RegisterTag>>>;

    Index<runir::kr::mpg::dl::Constructor<runir::kr::dl::ConceptTag>> index;
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
struct Data<runir::kr::mpg::dl::Constructor<runir::kr::dl::RoleTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>>;

    Index<runir::kr::mpg::dl::Constructor<runir::kr::dl::RoleTag>> index;
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
struct Data<runir::kr::mpg::dl::Constructor<runir::kr::dl::BooleanTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>>;

    Index<runir::kr::mpg::dl::Constructor<runir::kr::dl::BooleanTag>> index;
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
struct Data<runir::kr::mpg::dl::Constructor<runir::kr::dl::NumericalTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>>>;

    Index<runir::kr::mpg::dl::Constructor<runir::kr::dl::NumericalTag>> index;
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
