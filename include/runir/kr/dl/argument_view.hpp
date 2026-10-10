#ifndef RUNIR_KR_DL_ARGUMENT_VIEW_HPP_
#define RUNIR_KR_DL_ARGUMENT_VIEW_HPP_

#include "runir/kr/dl/argument_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::CategoryTag Category, formalism::SymbolContextFor<runir::kr::dl::Argument<Category>> C>
class View<Index<runir::kr::dl::Argument<Category>>, C> : public ygg::IndexViewBase<runir::kr::dl::Argument<Category>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::Argument<Category>, C>::IndexViewBase;

    const auto& get_name() const noexcept { return this->get_data().name; }
    auto get_identifier() const noexcept { return this->get_data().identifier; }
};

}  // namespace ygg

#endif
