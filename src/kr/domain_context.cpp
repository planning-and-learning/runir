#include "runir/kr/domain_context.hpp"

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/icp/repository.hpp"
#include "runir/kr/uns/repository.hpp"

#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr
{

DomainContextPtr DomainContext::create(const tyr::formalism::planning::PlanningDomain& domain) { return DomainContextPtr(new DomainContext(domain)); }

DomainContext::DomainContext(const tyr::formalism::planning::PlanningDomain& domain) :
    base_repository(runir::kr::ps::base::RepositoryFactory().create(runir::kr::dl::BaseConstructorRepositoryFactory().create(domain.get_repository()))),
    ext_repository(runir::kr::ps::ext::RepositoryFactory().create(runir::kr::dl::ExtConstructorRepositoryFactory().create(domain.get_repository()))),
    icp_repository(runir::kr::ps::icp::RepositoryFactory().create(ext_repository->get_dl_repository_ptr())),
    uns_repository(runir::kr::uns::RepositoryFactory().create(runir::kr::dl::UnsConstructorRepositoryFactory().create(domain.get_repository())))
{
}

}  // namespace runir::kr
