#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/grammar_view.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::base
{

void bind_grammar_grammar(nb::module_& m)
{
    using View = ygg::View<ygg::Index<runir::kr::dl::grammar::GrammarTag<runir::kr::BaseFamilyTag>>,
                           runir::kr::dl::grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>;

    auto cls = nb::class_<View>(m, "Grammar").def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace runir::kr::dl::base
