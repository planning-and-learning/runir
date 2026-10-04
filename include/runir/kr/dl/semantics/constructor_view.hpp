#ifndef RUNIR_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/constructor_index.hpp"
#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/boolean_view.hpp"
#include "runir/kr/dl/semantics/concept_view.hpp"
#include "runir/kr/dl/semantics/numerical_view.hpp"
#include "runir/kr/dl/semantics/role_view.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::FamilyConstructor<Family, Category>> C>
class View<Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C> :
    public formalism::detail::View<Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C>
{
public:
    View(Index<runir::kr::dl::FamilyConstructor<Family, Category>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C>(handle, context)
    {
    }

    auto get_variant() const noexcept { return make_view(this->get_data().variant, *this->m_context); }
    bool is_static() const noexcept { return this->get_data().is_static; }
};

}

#endif
