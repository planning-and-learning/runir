#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"

#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/ps/dl/evaluation.hpp>
#include <runir/kr/uns/repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::uns::dl
{

void bind_boolean_feature(nb::module_& m, RepositoryBinding& repository)
{
    using Feature = runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>;
    using ConcreteFeature = runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>;
    urd::python::bind_index<Feature>(m);
    urd::python::bind_data<Feature>(m);
    urd::python::bind_index<ConcreteFeature>(m);
    urd::python::bind_data<ConcreteFeature>(m);
    runir::kr::python::bind_feature_views<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>(m, repository);
    runir::kr::python::bind_feature_evaluate<urd::index_view<Feature, Repository>>(m);
    runir::kr::python::bind_feature_evaluate<urd::index_view<ConcreteFeature, Repository>>(m);
}

}  // namespace runir::kr::uns::dl
