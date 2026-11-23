#ifndef PYRUNIR_CONVERSIONS_HPP_
#define PYRUNIR_CONVERSIONS_HPP_

// The conversions from Python of the children of the contexts pyrunir binds builders of (urd::python::detail::
// conversions), but the queries' and the denotations', whose builders' fields are no constant expression (which a
// generic clone needs) and which are converted where their builders are bound: declared here, instantiated once in conversions_<context>.cpp, so that a source
// that binds a builder compiles its own kind's conversions only, not those of every kind its children reach (brokkr's template rule T5).

#include "pyrunir/module.hpp"

#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/control_repository.hpp>
#include <runir/kr/ps/ext/control_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/icp/control_view.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/uns/repository.hpp>
#include <urd/python/conversion_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::python
{
/// The kinds of a symbol context.
template<typename Context>
using KindsOf = typename ygg::formalism::detail::SymbolContextOf<Context>::SymbolTypes;

using DlContext = runir::kr::dl::ConstructorRepository;
using DlKinds = KindsOf<DlContext>;
using GrammarContext = runir::kr::dl::grammar::ConstructorRepository;
using GrammarKinds = KindsOf<GrammarContext>;
using CnfGrammarContext = runir::kr::dl::cnf_grammar::ConstructorRepository;
using CnfGrammarKinds = KindsOf<CnfGrammarContext>;
using BaseContext = runir::kr::ps::base::Repository;
using BaseKinds = KindsOf<BaseContext>;
using ExtContext = runir::kr::ps::ext::Repository;
using ExtKinds = KindsOf<ExtContext>;
using IcpContext = runir::kr::ps::icp::Repository;
using IcpKinds = KindsOf<IcpContext>;
using UnsContext = runir::kr::uns::Repository;
using UnsKinds = KindsOf<UnsContext>;
using ExtControlContext = runir::kr::ps::ext::ControlRepository;
using ExtControlKinds = KindsOf<ExtControlContext>;
using IcpControlContext = runir::kr::ps::icp::ControlRepository;
using IcpControlKinds = KindsOf<IcpControlContext>;
}  // namespace runir::python

URD_PYTHON_CONVERSIONS(extern, runir::python::DlContext, runir::python::DlKinds, 114)
URD_PYTHON_CONVERSIONS(extern, runir::python::GrammarContext, runir::python::GrammarKinds, 58)
URD_PYTHON_CONVERSIONS(extern, runir::python::CnfGrammarContext, runir::python::CnfGrammarKinds, 62)
URD_PYTHON_CONVERSIONS(extern, runir::python::BaseContext, runir::python::BaseKinds, 16)
URD_PYTHON_CONVERSIONS(extern, runir::python::ExtContext, runir::python::ExtKinds, 51)
URD_PYTHON_CONVERSIONS(extern, runir::python::IcpContext, runir::python::IcpKinds, 30)
URD_PYTHON_CONVERSIONS(extern, runir::python::UnsContext, runir::python::UnsKinds, 5)
URD_PYTHON_CONVERSIONS(extern, runir::python::ExtControlContext, runir::python::ExtControlKinds, 2)
URD_PYTHON_CONVERSIONS(extern, runir::python::IcpControlContext, runir::python::IcpControlKinds, 1)

#endif
