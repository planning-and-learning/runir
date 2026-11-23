#ifndef PYRUNIR_KR_PS_EXT_CONVERSIONS_HPP_
#define PYRUNIR_KR_PS_EXT_CONVERSIONS_HPP_

// The conversions from Python of the builders of the ext family's programs and of its execution control's records: declared here, instantiated once in
// conversions.cpp, so that a source that binds a builder compiles its own kind's conversions only, not those of every kind its children reach (brokkr's
// template rule T5).

#include "pyrunir/module.hpp"

#include <runir/kr/ps/control_repository.hpp>
#include <runir/kr/ps/ext/control_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <urd/python/conversion_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

URD_PYTHON_CONVERSIONS(extern, runir::kr::ps::ext::Repository, ygg::formalism::symbol_types_t<runir::kr::ps::ext::Repository>, 51)
URD_PYTHON_CONVERSIONS(extern, runir::kr::ps::ext::ControlRepository, ygg::formalism::symbol_types_t<runir::kr::ps::ext::ControlRepository>, 2)

#endif
