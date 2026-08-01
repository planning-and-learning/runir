#ifndef PYRUNIR_KR_UNS_DL_BINDINGS_HPP_
#define PYRUNIR_KR_UNS_DL_BINDINGS_HPP_

#include "module.hpp"

namespace runir::kr::uns::dl
{

void bind_feature(nb::module_& m);
void bind_parser(nb::module_& m);
void bind_classifier_factory(nb::module_& m);

}  // namespace runir::kr::uns::dl

#endif
