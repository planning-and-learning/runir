#ifndef RUNIR_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/record_view.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<runir::kr::dl::CategoryTag Category, typename V>
struct view_mixin<runir::kr::dl::Constructor<Category>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_variant() const noexcept { return self().template get<&urd::record_of<V>::variant>(); }
    [[nodiscard]] bool is_static() const noexcept { return self().data().is_static; }
};
}  // namespace urd

#endif
