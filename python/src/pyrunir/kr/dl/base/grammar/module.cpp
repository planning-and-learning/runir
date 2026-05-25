#include "module.hpp"

namespace runir::kr::dl::base
{

void bind_grammar_module_definitions(nb::module_& m)
{
    bind_grammar_indices(m);
    bind_grammar_datas(m);
    bind_grammar_views(m);
    bind_grammar_constructor_repository(m);
    bind_grammar_grammar(m);
    bind_grammar_factory(m);
}

}  // namespace runir::kr::dl::base
