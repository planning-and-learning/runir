#ifndef RUNIR_KR_PS_ICP_MODULE_VIEW_HPP_
#define RUNIR_KR_PS_ICP_MODULE_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/ps/icp/module_data.hpp"
#include "runir/kr/ps/module_view.hpp"

#include <concepts>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::icp::Module, V> : runir::kr::ps::ModuleViewMixin<V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_reset_pairs() const noexcept { return self().data().reset_pairs; }
    auto get_concept_features() const noexcept { return this->template get_features<runir::kr::dl::ConceptTag>(); }
};
}  // namespace urd

#endif
