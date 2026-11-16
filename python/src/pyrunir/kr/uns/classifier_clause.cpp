#include "bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/uns/classifier_view.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <urd/elements/flat.hpp>
#include <urd/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::uns
{

void bind_classifier_clause(nb::module_& m)
{
    urd::python::bind_data<ClassifierClause>(m);
    urd::python::bind_view<ClassifierClauseView>(m);
}

}  // namespace runir::kr::uns
