#include "pyrunir/kr/dl/serialization.hpp"

#include "module.hpp"

#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>

namespace runir::kr::uns
{
void bind_serialization() { ygg::python::bind_serialization(ygg::python::index_views<runir::kr::uns::Repository, runir::kr::uns::RepositoryTypes> {}); }
}  // namespace runir::kr::uns
