#ifndef RUNIR_KR_MPG_DL_POLICY_FACTORY_HPP_
#define RUNIR_KR_MPG_DL_POLICY_FACTORY_HPP_

#include "runir/kr/mpg/repository.hpp"

#include <string>

namespace runir::kr::mpg::dl
{

enum class PolicySpecification
{
};

class PolicyFactory
{
public:
    static PolicyView create_empty(Repository& repository);
    static std::string create_empty_description();
};

}  // namespace runir::kr::mpg::dl

#endif
