#ifndef PYRUNIR_KR_PS_BASE_CONVERSIONS_HPP_
#define PYRUNIR_KR_PS_BASE_CONVERSIONS_HPP_

// The conversions from Python of the builders of the policy sketches: declared here, instantiated once in conversions.cpp, so that a source that binds a
// builder compiles its own kind's conversions only, not those of every kind its children reach (brokkr's template rule T5).

#include "pyrunir/module.hpp"

#include <runir/kr/ps/base/repository.hpp>
#include <urd/python/conversion_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

URD_PYTHON_CONVERSIONS(extern, runir::kr::ps::base::Repository, ygg::formalism::symbol_types_t<runir::kr::ps::base::Repository>, 16)

#endif
