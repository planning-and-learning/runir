#include "dl.hpp"

#include "binding_helpers.hpp"

#include <runir/knowledge_representation/dl/semantics/datas.hpp>

namespace runir::kr::dl
{

void bind_datas(nb::module_& m)
{
    python_detail::bind_constructor_datas<runir::kr::dl::Concept,
                                          runir::kr::dl::Role,
                                          runir::kr::dl::Boolean,
                                          runir::kr::dl::Numerical,
                                          runir::kr::dl::Constructor>(m);
}

}  // namespace runir::kr::dl
