#include "pyrunir/kr/ps/base/module.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/syntactic_complexity.hpp>

namespace runir::kr::ps::base
{

void bind_syntactic_complexity(nb::module_& m) { m.def("syntactic_complexity", &runir::kr::ps::base::syntactic_complexity<Repository>, nb::arg("sketch")); }

}  // namespace runir::kr::ps::base
