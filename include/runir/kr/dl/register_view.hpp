#ifndef RUNIR_KR_DL_REGISTER_VIEW_HPP_
#define RUNIR_KR_DL_REGISTER_VIEW_HPP_

#include "runir/kr/dl/register_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::dl::ConceptOrRoleTag Category, formalism::SymbolContextFor<runir::kr::dl::Register<Category>> C>
class View<Index<runir::kr::dl::Register<Category>>, C> : public formalism::detail::View<Index<runir::kr::dl::Register<Category>>, C>
{
public:
    View(Index<runir::kr::dl::Register<Category>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::dl::Register<Category>>, C>(handle, context)
    {
    }

    const auto& get_name() const noexcept { return this->get_data().name; }
    auto get_identifier() const noexcept { return this->get_data().identifier; }
};

}  // namespace ygg

#endif
