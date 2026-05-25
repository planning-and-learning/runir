#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/base/repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/grammar_view.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl::base
{

void bind_grammar_grammar(nb::module_& m)
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::GrammarTag<runir::kr::BaseFamilyTag>>, runir::kr::dl::grammar::base::ConstructorRepository>;

    auto cls = nb::class_<View>(m, "Grammar").def("get_index", &View::get_index);
    tyr::add_print(cls);
    tyr::add_hash(cls);
}

}  // namespace runir::kr::dl::base
