#ifndef RUNIR_KR_PS_EXT_DL_PARSER_HPP_
#define RUNIR_KR_PS_EXT_DL_PARSER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"

#include <string>
#include <tyr/formalism/planning/declarations.hpp>
#include <vector>

namespace runir::kr::ps::ext::dl
{

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>
parse_concept(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository);

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>
parse_role(const std::string& description,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository);

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>
parse_boolean(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository);

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>
parse_numerical(const std::string& description,
                tyr::formalism::planning::DomainView domain,
                runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository);

ModuleView parse_module(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository);

ModuleProgramView parse_module_program(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository);

std::vector<ModuleView> parse_modules(const std::vector<std::string>& descriptions, tyr::formalism::planning::DomainView domain, Repository& repository);

}  // namespace runir::kr::ps::ext::dl

#endif
