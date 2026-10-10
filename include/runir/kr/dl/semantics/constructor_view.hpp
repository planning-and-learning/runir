#ifndef RUNIR_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/boolean_view.hpp"
#include "runir/kr/dl/semantics/concept_view.hpp"
#include "runir/kr/dl/semantics/numerical_view.hpp"
#include "runir/kr/dl/semantics/role_view.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::FamilyConstructor<Family, Category>> C>
class View<Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C> :
    public ygg::IndexViewBase<runir::kr::dl::FamilyConstructor<Family, Category>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::FamilyConstructor<Family, Category>, C>::IndexViewBase;

    auto get_variant() const noexcept { return make_view(this->get_data().variant, this->get_context()); }
    bool is_static() const noexcept { return this->get_data().is_static; }
};

}

#endif
