#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/grammar_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_grammar_grammar(nb::module_& m)
{
    using Repository = runir::kr::dl::grammar::ConstructorRepository;
    urd::python::bind_data<runir::kr::dl::grammar::DerivationRule<ConceptTag>>(m);
    urd::python::bind_data<runir::kr::dl::grammar::DerivationRule<RoleTag>>(m);
    urd::python::bind_data<runir::kr::dl::grammar::DerivationRule<BooleanTag>>(m);
    urd::python::bind_data<runir::kr::dl::grammar::DerivationRule<NumericalTag>>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::grammar::DerivationRule<ConceptTag>, Repository>>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::grammar::DerivationRule<RoleTag>, Repository>>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::grammar::DerivationRule<BooleanTag>, Repository>>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::grammar::DerivationRule<NumericalTag>, Repository>>(m);
    urd::python::bind_data<runir::kr::dl::grammar::GrammarTag>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::grammar::GrammarTag, Repository>>(m);
}

}  // namespace runir::kr::dl
