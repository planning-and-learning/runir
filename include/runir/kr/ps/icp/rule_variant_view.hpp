#ifndef RUNIR_KR_PS_ICP_RULE_VARIANT_VIEW_HPP_
#define RUNIR_KR_PS_ICP_RULE_VARIANT_VIEW_HPP_

#include "runir/kr/ps/icp/rule_variant_data.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>> C>
class View<Index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>, C> : public ygg::IndexViewBase<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>, C>::IndexViewBase;

    const auto& get_symbol() const noexcept { return this->get_data().symbol; }
    auto get_variant() const noexcept { return make_view(this->get_data().variant, this->get_context()); }
};

}  // namespace ygg

#endif
