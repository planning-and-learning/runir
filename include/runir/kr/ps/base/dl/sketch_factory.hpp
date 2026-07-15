#ifndef RUNIR_KR_PS_BASE_DL_SKETCH_FACTORY_HPP_
#define RUNIR_KR_PS_BASE_DL_SKETCH_FACTORY_HPP_

#include "runir/kr/ps/base/declarations.hpp"

#include <string>
#include <tyr/formalism/planning/declarations.hpp>

namespace runir::kr::ps::base::dl
{

enum class SketchSpecification
{
    GRIPPER_FRANCE_ET_AL_AAAI2021,
    BLOCKS3OPS_FRANCE_ET_AL_AAAI2021,
    SPANNER_FRANCE_ET_AL_AAAI2021,
    DELIVERY_FRANCE_ET_AL_AAAI2021,
};

class SketchFactory
{
public:
    static SketchView create_empty(Repository& repository);

    static SketchView create(SketchSpecification specification, tyr::formalism::planning::DomainView domain, Repository& repository);

    static std::string create_empty_description();

    static std::string create_description(SketchSpecification specification);

    static SketchView create_gripper_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository);
    static SketchView create_blocks3ops_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository);
    static SketchView create_spanner_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository);
    static SketchView create_delivery_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository);

    static std::string create_gripper_france_et_al_aaai2021_description();
    static std::string create_blocks3ops_france_et_al_aaai2021_description();
    static std::string create_spanner_france_et_al_aaai2021_description();
    static std::string create_delivery_france_et_al_aaai2021_description();
};

}  // namespace runir::kr::ps::base::dl

#endif
