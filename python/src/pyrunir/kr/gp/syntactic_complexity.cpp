#include "module.hpp"

#include <runir/kr/gp/repository.hpp>
#include <runir/kr/gp/syntactic_complexity.hpp>

namespace runir::kr::gp
{

void bind_syntactic_complexity(nb::module_& m) { m.def("syntactic_complexity", &runir::kr::gp::syntactic_complexity<Repository>, nb::arg("policy")); }

}  // namespace runir::kr::gp
