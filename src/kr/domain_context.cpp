#include "runir/kr/domain_context.hpp"

#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/icp/repository.hpp"
#include "runir/kr/uns/repository.hpp"

#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr
{

DomainContextPtr DomainContext::create(const tyr::formalism::planning::PlanningDomain& domain) { return DomainContextPtr(new DomainContext(domain)); }

DomainContext::DomainContext(const tyr::formalism::planning::PlanningDomain& domain) :
    ext_repository(runir::kr::ps::ext::RepositoryFactory().create(runir::kr::dl::ConstructorRepositoryFactory().create(domain.get_repository()))),
    base_repository(runir::kr::ps::base::RepositoryFactory().create(ext_repository->get_dl_repository_ptr())),
    icp_repository(runir::kr::ps::icp::RepositoryFactory().create(ext_repository->get_dl_repository_ptr())),
    uns_repository(runir::kr::uns::RepositoryFactory().create(ext_repository->get_dl_repository_ptr()))
{
}

}  // namespace runir::kr
