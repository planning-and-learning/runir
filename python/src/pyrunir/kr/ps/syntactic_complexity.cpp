#include "module.hpp"

#include <runir/kr/ps/repository.hpp>
#include <runir/kr/ps/syntactic_complexity.hpp>

namespace runir::kr::ps
{

void bind_syntactic_complexity(nb::module_& m) { m.def("syntactic_complexity", &runir::kr::ps::syntactic_complexity<Repository>, nb::arg("sketch")); }

}  // namespace runir::kr::ps
