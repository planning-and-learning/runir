#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/knowledge_representation/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/knowledge_representation/dl/cnf_grammar/formatter.hpp>
#include <runir/knowledge_representation/dl/cnf_grammar/grammar_view.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

namespace cnf = runir::kr::dl::cnf_grammar;

void bind_cnf_grammar_grammar(nb::module_& m)
{
    using View = tyr::View<tyr::Index<cnf::GrammarTag>, cnf::ConstructorRepository>;

    auto cls = nb::class_<View>(m, "Grammar").def("get_index", &View::get_index);
    tyr::add_print(cls);
    tyr::add_hash(cls);
}

}  // namespace runir::kr::dl
