#include "bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/constructor_view.hpp>
#include <runir/kr/dl/semantics/uns/evaluation.hpp>
#include <runir/kr/dl/semantics/uns/evaluation_context.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::uns
{
namespace
{

template<CategoryTag Category>
void bind_constructor_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<Constructor<runir::kr::UnsFamilyTag, Category>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(cls);
}

template<CategoryTag Category>
void bind_constructor_view(nb::module_& m, const char* name)
{
    using Type = Constructor<runir::kr::UnsFamilyTag, Category>;
    using View = ygg::View<ygg::Index<Type>, UnsConstructorRepository>;
    using GroundContext = semantics::EvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>;
    using LiftedContext = semantics::EvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    cls.def("evaluate", [](View view, GroundContext& context) { return semantics::evaluate(view, context); }, nb::arg("context"))
        .def("evaluate", [](View view, LiftedContext& context) { return semantics::evaluate(view, context); }, nb::arg("context"))
        .def("syntactic_complexity", [](View view) { return semantics::syntactic_complexity(view); });
    m.def("syntactic_complexity", [](View view) { return semantics::syntactic_complexity(view); }, nb::arg("constructor"));
}

}  // namespace

void bind_semantics_constructor(nb::module_& m)
{
    bind_constructor_data<ConceptTag>(m, "ConceptData");
    bind_constructor_data<RoleTag>(m, "RoleData");
    bind_constructor_data<BooleanTag>(m, "BooleanData");
    bind_constructor_data<NumericalTag>(m, "NumericalData");

    bind_constructor_view<ConceptTag>(m, "Concept");
    bind_constructor_view<RoleTag>(m, "Role");
    bind_constructor_view<BooleanTag>(m, "Boolean");
    bind_constructor_view<NumericalTag>(m, "Numerical");
}

}  // namespace runir::kr::dl::uns
