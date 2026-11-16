#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/evaluation.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::ps::base::dl
{

void bind_numerical_feature(nb::module_& m, RepositoryBinding& repository)
{
    using Feature = runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>;
    using ConcreteFeature = runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>;
    urd::python::bind_data<Feature>(m);
    urd::python::bind_data<ConcreteFeature>(m);
    runir::kr::python::bind_feature_views<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>(m, repository);
    runir::kr::python::bind_feature_evaluate<urd::index_view<Feature, Repository>>(m);
    runir::kr::python::bind_feature_evaluate<urd::index_view<ConcreteFeature, Repository>>(m);
}

}  // namespace runir::kr::ps::base::dl
