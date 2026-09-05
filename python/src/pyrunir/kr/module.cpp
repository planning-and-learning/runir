#include "module.hpp"

#include "pyrunir/kr/dl/module.hpp"
#include "pyrunir/kr/ps/module.hpp"
#include "pyrunir/kr/uns/module.hpp"
#include "runir/kr/errors.hpp"

#include <yggdrasil/python/diagnostics.hpp>

namespace runir::kr
{
namespace
{

void bind_errors(nb::module_& m)
{
    auto semantic_error = ygg::bind_diagnostic_exception<SemanticError>(m, "SemanticError");
    ygg::bind_diagnostic_exception<ParseError>(m, "ParseError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<UndefinedSymbolError>(m, "UndefinedSymbolError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<DuplicateDefinitionError>(m, "DuplicateDefinitionError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<ArityMismatchError>(m, "ArityMismatchError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<InvalidExpressionError>(m, "InvalidExpressionError", semantic_error.ptr());
}

}  // namespace

void bind_module_definitions(nb::module_& m)
{
    bind_errors(m);
    bind_domain_context(m);
    bind_task_context(m);
    auto dl = m.def_submodule("dl");
    runir::kr::dl::bind_module_definitions(dl);

    auto ps = m.def_submodule("ps");
    runir::kr::ps::bind_module_definitions(ps);

    auto uns = m.def_submodule("uns");
    runir::kr::uns::bind_module_definitions(uns);
}

}  // namespace runir::kr
