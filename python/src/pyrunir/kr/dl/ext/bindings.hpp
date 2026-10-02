#ifndef PYRUNIR_KR_DL_EXT_BINDINGS_HPP_
#define PYRUNIR_KR_DL_EXT_BINDINGS_HPP_

#include "module.hpp"

#include <runir/kr/dl/declarations.hpp>

namespace runir::kr::dl::ext
{

void bind_argument(nb::module_& m);
void bind_register(nb::module_& m);
void bind_constructor(nb::module_& m);
void bind_parser(nb::module_& m);

}  // namespace runir::kr::dl::ext

#endif
