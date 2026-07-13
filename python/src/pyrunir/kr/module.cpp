#include "module.hpp"

#include "pyrunir/kr/dl/module.hpp"
#include "pyrunir/kr/ps/module.hpp"
#include "pyrunir/kr/uns/module.hpp"
#include "runir/kr/errors.hpp"

namespace runir::kr
{
namespace
{

void bind_errors(nb::module_& m)
{
    auto semantic_error = nb::exception<SemanticError>(m, "SemanticError");
    nb::exception<ParseError>(m, "ParseError", semantic_error.ptr());
    nb::exception<UndefinedSymbolError>(m, "UndefinedSymbolError", semantic_error.ptr());
    nb::exception<DuplicateDefinitionError>(m, "DuplicateDefinitionError", semantic_error.ptr());
    nb::exception<ArityMismatchError>(m, "ArityMismatchError", semantic_error.ptr());
    nb::exception<InvalidExpressionError>(m, "InvalidExpressionError", semantic_error.ptr());
}

}  // namespace

void bind_module_definitions(nb::module_& m)
{
    bind_errors(m);
    bind_task_context(m);
    auto dl = m.def_submodule("dl");
    runir::kr::dl::bind_module_definitions(dl);

    auto ps = m.def_submodule("ps");
    runir::kr::ps::bind_module_definitions(ps);

    auto uns = m.def_submodule("uns");
    runir::kr::uns::bind_module_definitions(uns);
}

}  // namespace runir::kr
