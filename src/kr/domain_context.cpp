#include "runir/kr/domain_context.hpp"

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/icp/repository.hpp"
#include "runir/kr/uns/repository.hpp"

#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr
{

DomainContextPtr DomainContext::create(const tyr::formalism::PlanningDomain& domain) { return DomainContextPtr(new DomainContext(domain)); }

DomainContext::DomainContext(const tyr::formalism::PlanningDomain& domain) :
    ext_repository(std::make_shared<runir::kr::ps::ext::Repository>(std::make_shared<runir::kr::dl::ConstructorRepository>(domain.get_repository()))),
    base_repository(std::make_shared<runir::kr::ps::base::Repository>(ext_repository->get_ptr<runir::kr::dl::ConstructorRepository>())),
    icp_repository(std::make_shared<runir::kr::ps::icp::Repository>(ext_repository->get_ptr<runir::kr::dl::ConstructorRepository>())),
    uns_repository(std::make_shared<runir::kr::uns::Repository>(ext_repository->get_ptr<runir::kr::dl::ConstructorRepository>()))
{
}

}  // namespace runir::kr
