#include "runir/kr/uns/dl/classifier_factory.hpp"

#include "runir/kr/uns/classifier_data.hpp"
#include "runir/kr/uns/repository.hpp"

#include <string>

namespace runir::kr::uns::dl
{

ClassifierView ClassifierFactory::create_empty(Repository& repository)
{
    auto data = ygg::Data<runir::kr::uns::Classifier>(std::string("c0"));
    return repository.get_or_create(data).first;
}

std::string ClassifierFactory::create_empty_description()
{
    return R"RUNIR((:classifier
    (:symbol c0)
    (:features)
    (:expression
        (or)
    )
))RUNIR";
}

}  // namespace runir::kr::uns::dl
