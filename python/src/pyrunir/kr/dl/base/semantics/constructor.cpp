#include "bindings.hpp"

#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/constructor_view.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
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
    auto cls = nb::class_<Data>(m, name)
                   .def(nb::init<>())
                   .def(nb::init<typename Data::Variant>(), nb::arg("variant"))
                   .def_rw("index", &Data::index)
                   .def_rw("variant", &Data::variant)
                   .def_rw("is_static", &Data::is_static);
    ygg::add_comparison(cls);
}

template<CategoryTag Category>
void bind_constructor_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, Category>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::BaseConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index).def("get_variant", &View::get_variant, nb::keep_alive<0, 1>());
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    runir::kr::python::bind_evaluate<runir::kr::BaseFamilyTag>(cls);
    cls.def("syntactic_complexity", [](View view) { return runir::kr::dl::semantics::syntactic_complexity(view); });
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
