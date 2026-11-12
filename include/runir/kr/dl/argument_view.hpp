#ifndef RUNIR_KR_DL_ARGUMENT_VIEW_HPP_
#define RUNIR_KR_DL_ARGUMENT_VIEW_HPP_

#include "runir/kr/dl/argument_data.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<runir::kr::dl::CategoryTag Category, typename V>
struct view_mixin<runir::kr::dl::Argument<Category>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_name() const noexcept { return self().data().name; }
    auto get_identifier() const noexcept { return self().data().identifier; }
};
}  // namespace urd

#endif
