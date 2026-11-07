#include "bindings.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/uns/classifier_view.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::uns
{

void bind_classifier_clause(nb::module_& m)
{
    using View = urd::index_view<ClassifierClause, Repository>;
    urd::python::bind_index<urd::index<ClassifierClause>>(m, "ClassifierClauseIndex");
    auto cls = nb::class_<View>(m, "ClassifierClause").def("index", &View::index).def("get_literals", &View::get_literals);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
}

}  // namespace runir::kr::uns
