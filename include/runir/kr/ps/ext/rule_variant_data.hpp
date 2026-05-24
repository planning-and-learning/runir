#ifndef RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_
#define RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_

#include "runir/kr/ps/ext/rule_index.hpp"
#include "runir/kr/ps/ext/rule_variant_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <utility>

namespace tyr
{

template<>
struct Data<runir::kr::ps::ext::RuleVariant>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag>>,
                                             Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>>,
                                             Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>>,
                                             Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>>;

    Index<runir::kr::ps::ext::RuleVariant> index;
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
