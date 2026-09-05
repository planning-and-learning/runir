#ifndef RUNIR_KR_DOMAIN_CONTEXT_HPP_
#define RUNIR_KR_DOMAIN_CONTEXT_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <tyr/formalism/planning/declarations.hpp>

namespace runir::kr
{

struct DomainContext
{
    const runir::kr::ps::base::RepositoryPtr base_repository;
    const runir::kr::ps::ext::RepositoryPtr ext_repository;
    const runir::kr::uns::RepositoryPtr uns_repository;

    static DomainContextPtr create(const tyr::formalism::planning::PlanningDomain& domain);

private:
    explicit DomainContext(const tyr::formalism::planning::PlanningDomain& domain);
};

}  // namespace runir::kr

#endif
