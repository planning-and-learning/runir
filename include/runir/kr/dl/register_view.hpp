#ifndef RUNIR_KR_DL_REGISTER_VIEW_HPP_
#define RUNIR_KR_DL_REGISTER_VIEW_HPP_

#include "runir/kr/dl/register_data.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<runir::kr::dl::ConceptOrRoleTag Category, typename V>
struct view_mixin<runir::kr::dl::Register<Category>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_name() const noexcept { return self().data().name; }
    auto get_identifier() const noexcept { return self().data().identifier; }
};
}  // namespace urd

#endif
