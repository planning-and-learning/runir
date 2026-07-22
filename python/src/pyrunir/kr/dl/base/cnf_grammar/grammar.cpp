#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/grammar_view.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::base
{

void bind_cnf_grammar_grammar(nb::module_& m)
{
    using View = ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Grammar<runir::kr::BaseFamilyTag>>,
                           runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>;

    auto cls = nb::class_<View>(m, "Grammar").def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace runir::kr::dl::base
