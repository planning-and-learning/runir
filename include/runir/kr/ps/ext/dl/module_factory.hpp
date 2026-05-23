#ifndef RUNIR_KR_PS_EXT_DL_MODULE_FACTORY_HPP_
#define RUNIR_KR_PS_EXT_DL_MODULE_FACTORY_HPP_

#include "runir/kr/ps/ext/repository.hpp"

#include <string>

namespace runir::kr::ps::ext::dl
{

enum class ModuleSpecification
{
};

class ModuleFactory
{
public:
    static ModuleView create_empty(Repository& repository);
    static std::string create_empty_description();
};

}  // namespace runir::kr::ps::ext::dl

#endif
