#ifndef RUNIR_KR_UNS_DL_CLASSIFIER_FACTORY_HPP_
#define RUNIR_KR_UNS_DL_CLASSIFIER_FACTORY_HPP_

#include "runir/kr/uns/declarations.hpp"

#include <string>

namespace runir::kr::uns::dl
{

class ClassifierFactory
{
public:
    static ClassifierView create_empty(Repository& repository);

    static std::string create_empty_description();
};

}  // namespace runir::kr::uns::dl

#endif
