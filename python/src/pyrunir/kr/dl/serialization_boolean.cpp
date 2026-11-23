#include "module.hpp"
#include "pyrunir/kr/dl/serialization.hpp"

#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/dl/repository.hpp>

namespace runir::kr::dl
{
void bind_boolean_serialization()
{ ygg::python::bind_serialization(ygg::python::index_views<runir::kr::dl::ConstructorRepository, runir::kr::dl::BooleanTypes> {}); }
}  // namespace runir::kr::dl
