#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/grammar_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_cnf_grammar_grammar(nb::module_& m)
{
    using Type = runir::kr::dl::cnf_grammar::Grammar;
    using View = urd::index_view<Type, runir::kr::dl::cnf_grammar::ConstructorRepository>;

    urd::python::bind_index<urd::index<Type>>(m, "GrammarIndex");
    auto cls = nb::class_<View>(m, "Grammar").def("get_index", &View::index);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
}

}  // namespace runir::kr::dl
