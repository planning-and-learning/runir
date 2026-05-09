#include "dl.hpp"

#include "binding_helpers.hpp"

#include <runir/knowledge_representation/dl/repository.hpp>
#include <runir/knowledge_representation/dl/semantics/formatter.hpp>
#include <runir/knowledge_representation/dl/semantics/views.hpp>

namespace runir::kr::dl
{

void bind_views(nb::module_& m)
{
    python_detail::bind_constructor_views<runir::kr::dl::Concept,
                                          runir::kr::dl::Role,
                                          runir::kr::dl::Boolean,
                                          runir::kr::dl::Numerical,
                                          runir::kr::dl::Constructor,
                                          runir::kr::dl::ConstructorRepository>(m);
}

}  // namespace runir::kr::dl
