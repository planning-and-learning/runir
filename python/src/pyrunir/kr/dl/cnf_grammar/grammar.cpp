#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/grammar_view.hpp>
#include <tyr/formalism/repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_cnf_grammar_grammar(nb::module_& m)
{
    using Repository = runir::kr::dl::cnf_grammar::ConstructorRepository;
    urd::python::bind_index<runir::kr::dl::cnf_grammar::DerivationRule<ConceptTag>>(m);
    urd::python::bind_data<runir::kr::dl::cnf_grammar::DerivationRule<ConceptTag>>(m);
    urd::python::bind_index<runir::kr::dl::cnf_grammar::DerivationRule<RoleTag>>(m);
    urd::python::bind_data<runir::kr::dl::cnf_grammar::DerivationRule<RoleTag>>(m);
    urd::python::bind_index<runir::kr::dl::cnf_grammar::DerivationRule<BooleanTag>>(m);
    urd::python::bind_data<runir::kr::dl::cnf_grammar::DerivationRule<BooleanTag>>(m);
    urd::python::bind_index<runir::kr::dl::cnf_grammar::DerivationRule<NumericalTag>>(m);
    urd::python::bind_data<runir::kr::dl::cnf_grammar::DerivationRule<NumericalTag>>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::cnf_grammar::DerivationRule<ConceptTag>, Repository>>(m);
    urd::python::bind_builder<runir::kr::dl::cnf_grammar::DerivationRule<ConceptTag>, Repository>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::cnf_grammar::DerivationRule<RoleTag>, Repository>>(m);
    urd::python::bind_builder<runir::kr::dl::cnf_grammar::DerivationRule<RoleTag>, Repository>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::cnf_grammar::DerivationRule<BooleanTag>, Repository>>(m);
    urd::python::bind_builder<runir::kr::dl::cnf_grammar::DerivationRule<BooleanTag>, Repository>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::cnf_grammar::DerivationRule<NumericalTag>, Repository>>(m);
    urd::python::bind_builder<runir::kr::dl::cnf_grammar::DerivationRule<NumericalTag>, Repository>(m);
    urd::python::bind_index<runir::kr::dl::cnf_grammar::SubstitutionRule<ConceptTag>>(m);
    urd::python::bind_data<runir::kr::dl::cnf_grammar::SubstitutionRule<ConceptTag>>(m);
    urd::python::bind_index<runir::kr::dl::cnf_grammar::SubstitutionRule<RoleTag>>(m);
    urd::python::bind_data<runir::kr::dl::cnf_grammar::SubstitutionRule<RoleTag>>(m);
    urd::python::bind_index<runir::kr::dl::cnf_grammar::SubstitutionRule<BooleanTag>>(m);
    urd::python::bind_data<runir::kr::dl::cnf_grammar::SubstitutionRule<BooleanTag>>(m);
    urd::python::bind_index<runir::kr::dl::cnf_grammar::SubstitutionRule<NumericalTag>>(m);
    urd::python::bind_data<runir::kr::dl::cnf_grammar::SubstitutionRule<NumericalTag>>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::cnf_grammar::SubstitutionRule<ConceptTag>, Repository>>(m);
    urd::python::bind_builder<runir::kr::dl::cnf_grammar::SubstitutionRule<ConceptTag>, Repository>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::cnf_grammar::SubstitutionRule<RoleTag>, Repository>>(m);
    urd::python::bind_builder<runir::kr::dl::cnf_grammar::SubstitutionRule<RoleTag>, Repository>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::cnf_grammar::SubstitutionRule<BooleanTag>, Repository>>(m);
    urd::python::bind_builder<runir::kr::dl::cnf_grammar::SubstitutionRule<BooleanTag>, Repository>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::cnf_grammar::SubstitutionRule<NumericalTag>, Repository>>(m);
    urd::python::bind_builder<runir::kr::dl::cnf_grammar::SubstitutionRule<NumericalTag>, Repository>(m);
    urd::python::bind_index<runir::kr::dl::cnf_grammar::Grammar>(m);
    urd::python::bind_data<runir::kr::dl::cnf_grammar::Grammar>(m);
    urd::python::bind_view<urd::index_view<runir::kr::dl::cnf_grammar::Grammar, Repository>>(m);
    urd::python::bind_builder<runir::kr::dl::cnf_grammar::Grammar, Repository>(m);
}

}  // namespace runir::kr::dl
