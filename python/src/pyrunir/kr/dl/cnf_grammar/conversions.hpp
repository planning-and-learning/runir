#ifndef PYRUNIR_KR_DL_CNF_GRAMMAR_CONVERSIONS_HPP_
#define PYRUNIR_KR_DL_CNF_GRAMMAR_CONVERSIONS_HPP_

// The conversions from Python of the builders of the CNF grammars: declared here, instantiated once in conversions.cpp, so that a source that binds a builder
// compiles its own kind's conversions only, not those of every kind its children reach (brokkr's template rule T5).

#include "pyrunir/module.hpp"

#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <urd/python/conversion_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

URD_PYTHON_CONVERSIONS(extern,
                       runir::kr::dl::cnf_grammar::ConstructorRepository,
                       ygg::formalism::symbol_types_t<runir::kr::dl::cnf_grammar::ConstructorRepository>,
                       62)

#endif
