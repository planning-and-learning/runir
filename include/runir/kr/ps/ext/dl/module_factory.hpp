#ifndef RUNIR_KR_PS_EXT_DL_MODULE_FACTORY_HPP_
#define RUNIR_KR_PS_EXT_DL_MODULE_FACTORY_HPP_

#include "runir/kr/ps/ext/repository.hpp"

#include <string>
#include <tyr/formalism/planning/domain_view.hpp>
#include <vector>

namespace runir::kr::ps::ext::dl
{

enum class ModuleSpecification
{
    ON_BONET_ET_AL_ICAPS2024,
    ON_TABLE_BONET_ET_AL_ICAPS2024,
    TOWER_BONET_ET_AL_ICAPS2024,
    BLOCKS_BONET_ET_AL_ICAPS2024,
};

class ModuleFactory
{
public:
    static ModuleView create_empty(Repository& repository);
    static ModuleView create(ModuleSpecification specification, tyr::formalism::planning::DomainView domain, Repository& repository);

    static std::string create_empty_description();
    static std::string create_description(ModuleSpecification specification);

    static std::vector<ModuleView> create_bonet_et_al_icaps2024_modules(tyr::formalism::planning::DomainView domain, Repository& repository);
    static std::vector<std::string> create_bonet_et_al_icaps2024_descriptions();
    static std::string create_bonet_et_al_icaps2024_program_description();
    static ModuleProgramView create_bonet_et_al_icaps2024_program(tyr::formalism::planning::DomainView domain, Repository& repository);

    static ModuleView create_on_bonet_et_al_icaps2024(tyr::formalism::planning::DomainView domain, Repository& repository);
    static ModuleView create_on_table_bonet_et_al_icaps2024(tyr::formalism::planning::DomainView domain, Repository& repository);
    static ModuleView create_tower_bonet_et_al_icaps2024(tyr::formalism::planning::DomainView domain, Repository& repository);
    static ModuleView create_blocks_bonet_et_al_icaps2024(tyr::formalism::planning::DomainView domain, Repository& repository);

    static std::string create_on_bonet_et_al_icaps2024_description();
    static std::string create_on_table_bonet_et_al_icaps2024_description();
    static std::string create_tower_bonet_et_al_icaps2024_description();
    static std::string create_blocks_bonet_et_al_icaps2024_description();
};

}  // namespace runir::kr::ps::ext::dl

#endif
