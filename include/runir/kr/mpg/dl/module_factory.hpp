#ifndef RUNIR_KR_MPG_DL_MODULE_FACTORY_HPP_
#define RUNIR_KR_MPG_DL_MODULE_FACTORY_HPP_

#include "runir/kr/mpg/repository.hpp"

#include <string>

namespace runir::kr::mpg::dl
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

}  // namespace runir::kr::mpg::dl

#endif
