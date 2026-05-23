#ifndef RUNIR_KR_MPG_RULE_VARIANT_DATA_HPP_
#define RUNIR_KR_MPG_RULE_VARIANT_DATA_HPP_

#include "runir/kr/mpg/rule_index.hpp"
#include "runir/kr/mpg/rule_variant_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <utility>

namespace tyr
{

template<>
struct Data<runir::kr::mpg::RuleVariant>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::mpg::Rule<runir::kr::mpg::SketchTag>>,
                                             Index<runir::kr::mpg::Rule<runir::kr::mpg::LoadTag>>,
                                             Index<runir::kr::mpg::Rule<runir::kr::mpg::DoTag>>,
                                             Index<runir::kr::mpg::Rule<runir::kr::mpg::CallTag>>>;

    Index<runir::kr::mpg::RuleVariant> index;
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
