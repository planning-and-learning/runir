#include "dl.hpp"

#include "binding_helpers.hpp"

#include <runir/knowledge_representation/dl/indices.hpp>
#include <runir/knowledge_representation/dl/cnf_grammar/indices.hpp>
#include <runir/knowledge_representation/dl/grammar/indices.hpp>

namespace runir::kr::dl
{

void bind_indices(nb::module_& m)
{
    python_detail::bind_constructor_indices<runir::kr::dl::Concept,
                                            runir::kr::dl::Role,
                                            runir::kr::dl::Boolean,
                                            runir::kr::dl::Numerical,
                                            runir::kr::dl::Constructor>(m);
}

}  // namespace runir::kr::dl
