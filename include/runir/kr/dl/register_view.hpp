#ifndef RUNIR_KR_DL_REGISTER_VIEW_HPP_
#define RUNIR_KR_DL_REGISTER_VIEW_HPP_

#include "runir/kr/dl/register_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::ConceptOrRoleTag Category, formalism::SymbolContextFor<runir::kr::dl::Register<Category>> C>
class View<Index<runir::kr::dl::Register<Category>>, C> : public ygg::IndexViewBase<runir::kr::dl::Register<Category>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::Register<Category>, C>::IndexViewBase;

    const auto& get_name() const noexcept { return this->get_data().name; }
    auto get_identifier() const noexcept { return this->get_data().identifier; }
};

}  // namespace ygg

#endif
