#include "bindings.hpp"
#include "pyrunir/kr/dl/conversions.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"
#include "pyrunir/kr/ps/icp/conversions.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/program_data.hpp>
#include <runir/kr/ps/icp/program_view.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/syntactic_complexity.hpp>
#include <urd/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::icp
{

void bind_program(nb::module_& m, RepositoryBinding& repository)
{
    urd::python::bind_index<Program>(m);
    urd::python::bind_data<Program>(m);
    urd::python::bind_view<ProgramView>(m);
    urd::python::bind_builder<Program, Repository>(m);
    runir::kr::python::bind_ps_optimize<runir::kr::IcpFamilyTag, Repository, ProgramView>(m);
    ygg::bind_insert<Program>(repository);
}

}  // namespace runir::kr::ps::icp
