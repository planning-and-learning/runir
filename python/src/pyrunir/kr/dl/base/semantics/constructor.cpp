#include "bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/base/evaluation_context.hpp>
#include <runir/kr/dl/semantics/constructor_view.hpp>
#include <runir/kr/dl/semantics/denotation_caches.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::base
{
namespace
{

template<CategoryTag Category>
void bind_constructor_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, Category>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(cls);
}

template<CategoryTag Category>
void bind_constructor_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, Category>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::BaseConstructorRepository>;
    using GroundContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>;
    using DenotationCaches = runir::kr::dl::semantics::DenotationCaches<runir::kr::BaseFamilyTag>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    cls.def(
           "evaluate",
           [](View view, GroundContext& context) { return runir::kr::dl::semantics::evaluate(view, context); },
           nb::arg("context"),
           nb::keep_alive<0, 2>())
        .def(
            "evaluate",
            [](View view, LiftedContext& context) { return runir::kr::dl::semantics::evaluate(view, context); },
            nb::arg("context"),
            nb::keep_alive<0, 2>())
        .def(
            "evaluate",
            [](View view, GroundContext& context, DenotationCaches& caches) { return runir::kr::dl::semantics::evaluate(view, context, caches); },
            nb::arg("context"),
            nb::arg("denotation_caches"),
            nb::keep_alive<0, 2>(),
            nb::keep_alive<3, 1>(),
            nb::keep_alive<3, 2>())
        .def(
            "evaluate",
            [](View view, LiftedContext& context, DenotationCaches& caches) { return runir::kr::dl::semantics::evaluate(view, context, caches); },
            nb::arg("context"),
            nb::arg("denotation_caches"),
            nb::keep_alive<0, 2>(),
            nb::keep_alive<3, 1>(),
            nb::keep_alive<3, 2>())
        .def("syntactic_complexity", [](View view) { return runir::kr::dl::semantics::syntactic_complexity(view); });
    m.def("syntactic_complexity", [](View view) { return runir::kr::dl::semantics::syntactic_complexity(view); }, nb::arg("constructor"));
}

}  // namespace

void bind_semantics_constructor(nb::module_& m)
{
    using Concept = runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, ConceptTag>;
    using Role = runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, RoleTag>;
    using Boolean = runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, BooleanTag>;
    using Numerical = runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, NumericalTag>;

    ygg::bind_index<ygg::Index<Concept>>(m, "ConceptIndex");
    ygg::bind_index<ygg::Index<Role>>(m, "RoleIndex");
    ygg::bind_index<ygg::Index<Boolean>>(m, "BooleanIndex");
    ygg::bind_index<ygg::Index<Numerical>>(m, "NumericalIndex");

    bind_constructor_data<ConceptTag>(m, "ConceptData");
    bind_constructor_data<RoleTag>(m, "RoleData");
    bind_constructor_data<BooleanTag>(m, "BooleanData");
    bind_constructor_data<NumericalTag>(m, "NumericalData");

    bind_constructor_view<ConceptTag>(m, "Concept");
    bind_constructor_view<RoleTag>(m, "Role");
    bind_constructor_view<BooleanTag>(m, "Boolean");
    bind_constructor_view<NumericalTag>(m, "Numerical");
}

}  // namespace runir::kr::dl::base
