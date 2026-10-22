#include "bindings.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/grammar_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_grammar_grammar(nb::module_& m)
{
    using Type = runir::kr::dl::grammar::GrammarTag;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::grammar::ConstructorRepository>;

    ygg::bind_index<ygg::Index<Type>>(m, "GrammarIndex");
    auto cls = nb::class_<View>(m, "Grammar").def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace runir::kr::dl
