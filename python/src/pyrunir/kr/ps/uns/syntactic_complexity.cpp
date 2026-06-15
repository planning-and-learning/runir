#include "pyrunir/kr/ps/uns/module.hpp"

#include <runir/kr/ps/uns/repository.hpp>
#include <runir/kr/ps/uns/syntactic_complexity.hpp>

namespace runir::kr::ps::uns
{

void bind_syntactic_complexity(nb::module_& m) { m.def("syntactic_complexity", &runir::kr::ps::uns::syntactic_complexity<Repository>, nb::arg("sketch")); }

}  // namespace runir::kr::ps::uns
