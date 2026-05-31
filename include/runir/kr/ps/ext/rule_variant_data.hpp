#ifndef RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_
#define RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_

#include "runir/kr/ps/ext/rule_index.hpp"
#include "runir/kr/ps/ext/rule_variant_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <utility>

namespace ygg
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
        ygg::clear(index);
        ygg::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace ygg

#endif
