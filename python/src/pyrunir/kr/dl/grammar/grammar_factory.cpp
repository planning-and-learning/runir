#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/grammar_factory.hpp>

namespace runir::kr::dl
{

void bind_grammar_factory(nb::module_& m)
{
    nb::enum_<runir::kr::dl::grammar::GrammarSpecification>(m, "GrammarSpecification")
        .value("FRANCE_ET_AL_AAAI2021", runir::kr::dl::grammar::GrammarSpecification::FRANCE_ET_AL_AAAI2021);

    nb::class_<runir::kr::dl::grammar::GrammarFactory>(m, "GrammarFactory")
        .def_static("create",
                    &runir::kr::dl::grammar::GrammarFactory::create,
                    nb::arg("specification"),
                    nb::arg("domain"),
                    nb::arg("repository"),
                    nb::keep_alive<0, 3>())
        .def_static("create_description", &runir::kr::dl::grammar::GrammarFactory::create_description, nb::arg("specification"), nb::arg("domain"))
        .def_static("create_france_et_al_aaai2021",
                    &runir::kr::dl::grammar::GrammarFactory::create_france_et_al_aaai2021,
                    nb::arg("domain"),
                    nb::arg("repository"),
                    nb::keep_alive<0, 2>())
        .def_static("create_france_et_al_aaai2021_description",
                    &runir::kr::dl::grammar::GrammarFactory::create_france_et_al_aaai2021_description,
                    nb::arg("domain"));
}

}  // namespace runir::kr::dl
