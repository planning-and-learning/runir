#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/knowledge_representation/dl/grammar/constructor_repository.hpp>
#include <runir/knowledge_representation/dl/grammar/formatter.hpp>
#include <runir/knowledge_representation/dl/grammar/grammar_view.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

namespace grammar = runir::kr::dl::grammar;

void bind_grammar_grammar(nb::module_& m)
{
    using View = tyr::View<tyr::Index<grammar::GrammarTag>, grammar::ConstructorRepository>;

    auto cls = nb::class_<View>(m, "Grammar").def("get_index", &View::get_index);
    tyr::add_print(cls);
    tyr::add_hash(cls);
}

}  // namespace runir::kr::dl
