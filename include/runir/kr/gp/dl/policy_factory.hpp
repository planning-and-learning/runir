#ifndef RUNIR_KR_GP_DL_POLICY_FACTORY_HPP_
#define RUNIR_KR_GP_DL_POLICY_FACTORY_HPP_

#include "runir/kr/gp/repository.hpp"

#include <string>
#include <tyr/formalism/planning/domain_view.hpp>

namespace runir::kr::gp::dl
{

enum class PolicySpecification
{
    GRIPPER_FRANCE_ET_AL_AAAI2021,
    BLOCKS3OPS_FRANCE_ET_AL_AAAI2021,
    SPANNER_FRANCE_ET_AL_AAAI2021,
    DELIVERY_FRANCE_ET_AL_AAAI2021,
};

class PolicyFactory
{
public:
    static PolicyView create(PolicySpecification specification, tyr::formalism::planning::DomainView domain, Repository& repository);

    static std::string create_description(PolicySpecification specification);

    static PolicyView create_gripper_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository);
    static PolicyView create_blocks3ops_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository);
    static PolicyView create_spanner_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository);
    static PolicyView create_delivery_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository);

    static std::string create_gripper_france_et_al_aaai2021_description();
    static std::string create_blocks3ops_france_et_al_aaai2021_description();
    static std::string create_spanner_france_et_al_aaai2021_description();
    static std::string create_delivery_france_et_al_aaai2021_description();
};

}  // namespace runir::kr::gp::dl

#endif
