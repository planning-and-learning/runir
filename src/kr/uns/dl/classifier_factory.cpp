#include "runir/kr/uns/dl/classifier_factory.hpp"

#include "runir/kr/uns/classifier_data.hpp"
#include "runir/kr/uns/repository.hpp"

#include <string>

namespace runir::kr::uns::dl
{

ClassifierView ClassifierFactory::create_empty(Repository& repository)
{
    auto builder = runir::kr::uns::Builder {};
    auto data = runir::kr::uns::checkout<runir::kr::uns::Classifier>(builder);
    data->symbol = "c0";
    return runir::kr::uns::get_or_create(repository, *data).first;
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
