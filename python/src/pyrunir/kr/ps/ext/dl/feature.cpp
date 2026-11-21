#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"

#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::ps::ext::dl
{

// One translation unit per kind: the three instantiations together need about 6 GB to compile.
void bind_features(nb::module_& m, RepositoryBinding& repository)
{
    urd::python::bind_index<runir::kr::ps::Feature<ExtFamilyTag, runir::kr::dl::ConceptTag>>(m);
    urd::python::bind_data<runir::kr::ps::Feature<ExtFamilyTag, runir::kr::dl::ConceptTag>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteFeature<ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteFeature<ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>>(m);
    urd::python::bind_index<runir::kr::ps::Feature<ExtFamilyTag, runir::kr::dl::RoleTag>>(m);
    urd::python::bind_data<runir::kr::ps::Feature<ExtFamilyTag, runir::kr::dl::RoleTag>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteFeature<ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteFeature<ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>>(m);
    urd::python::bind_index<runir::kr::ps::Feature<ExtFamilyTag, runir::kr::dl::BooleanTag>>(m);
    urd::python::bind_data<runir::kr::ps::Feature<ExtFamilyTag, runir::kr::dl::BooleanTag>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteFeature<ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteFeature<ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>(m);
    urd::python::bind_index<runir::kr::ps::Feature<ExtFamilyTag, runir::kr::dl::NumericalTag>>(m);
    urd::python::bind_data<runir::kr::ps::Feature<ExtFamilyTag, runir::kr::dl::NumericalTag>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteFeature<ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteFeature<ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>>(m);
    urd::python::bind_index<runir::kr::ps::Feature<ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>(m);
    urd::python::bind_data<runir::kr::ps::Feature<ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteFeature<ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::QueryFeature>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteFeature<ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::QueryFeature>>(m);
    runir::kr::python::bind_feature_views<ExtFamilyTag, runir::kr::dl::ConceptTag>(m, repository);
    runir::kr::python::bind_feature_views<ExtFamilyTag, runir::kr::dl::RoleTag>(m, repository);
    runir::kr::python::bind_feature_views<ExtFamilyTag, runir::kr::dl::BooleanTag>(m, repository);
    runir::kr::python::bind_feature_views<ExtFamilyTag, runir::kr::dl::NumericalTag>(m, repository);
    runir::kr::python::bind_feature_views<ExtFamilyTag, runir::kr::ps::dl::QueryFeature>(m, repository);
}

}  // namespace runir::kr::ps::ext::dl
