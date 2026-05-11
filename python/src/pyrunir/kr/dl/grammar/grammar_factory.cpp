#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/grammar_factory.hpp>

namespace runir::kr::dl
{

namespace grammar = runir::kr::dl::grammar;

using namespace nanobind::literals;

void bind_grammar_factory(nb::module_& m)
{
    nb::enum_<grammar::GrammarSpecification>(m, "GrammarSpecification").value("FRANCE_ET_AL_AAAI2021", grammar::GrammarSpecification::FRANCE_ET_AL_AAAI2021);

    nb::class_<grammar::GrammarFactory>(m, "GrammarFactory")
        .def_static("create", &grammar::GrammarFactory::create, "specification"_a, "domain"_a, "repository"_a, nb::keep_alive<0, 3>())
        .def_static("create_description", &grammar::GrammarFactory::create_description, "specification"_a, "domain"_a)
        .def_static("create_france_et_al_aaai2021", &grammar::GrammarFactory::create_france_et_al_aaai2021, "domain"_a, "repository"_a, nb::keep_alive<0, 2>())
        .def_static("create_france_et_al_aaai2021_description", &grammar::GrammarFactory::create_france_et_al_aaai2021_description, "domain"_a);
}

}  // namespace runir::kr::dl
