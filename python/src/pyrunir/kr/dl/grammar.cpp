#include "dl.hpp"

#include "binding_helpers.hpp"

#include <memory>
#include <runir/knowledge_representation/dl/grammar/datas.hpp>
#include <runir/knowledge_representation/dl/grammar/formatter.hpp>
#include <runir/knowledge_representation/dl/grammar/parser.hpp>
#include <runir/knowledge_representation/dl/grammar/views.hpp>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::kr::dl
{

namespace grammar = runir::kr::dl::grammar;
namespace fp = tyr::formalism::planning;

using namespace nanobind::literals;

void bind_grammar_module_definitions(nb::module_& m)
{
    python_detail::bind_constructor_indices<grammar::Concept, grammar::Role, grammar::Boolean, grammar::Numerical, grammar::Constructor>(m);
    python_detail::bind_constructor_datas<grammar::Concept, grammar::Role, grammar::Boolean, grammar::Numerical, grammar::Constructor>(m);
    python_detail::bind_constructor_views<grammar::Concept, grammar::Role, grammar::Boolean, grammar::Numerical, grammar::Constructor, grammar::ConstructorRepository>(m);

    auto repository = nb::class_<grammar::ConstructorRepository>(m, "ConstructorRepository");
    repository.def(nb::new_([](size_t index, std::shared_ptr<const fp::Repository> planning_repository)
                            { return std::make_unique<grammar::ConstructorRepository>(index, std::move(planning_repository)); }),
                   "index"_a,
                   "planning_repository"_a);
    python_detail::bind_clearable_repository(repository);

    python_detail::bind_view<grammar::GrammarTag, grammar::ConstructorRepository>(m, "Grammar");

    m.def("parse", &grammar::parse_grammar, "description"_a, "domain"_a, "repository"_a, nb::keep_alive<0, 3>());
}

}  // namespace runir::kr::dl
