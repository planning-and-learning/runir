#include "../data_bindings.hpp"
#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/numerical_view.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::dl
{
namespace
{

template<typename Tag>
void bind_numerical_data(nb::module_& m, const char* name)
{
    using Data = urd::data<Numerical<Tag>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    urd::python::add_comparison(cls);
    python::def_numerical_data_constructors<Tag,
                                            urd::index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>,
                                            urd::index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>,
                                            urd::index<runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>>,
                                            runir::kr::dl::ConstructorRepository>(cls);

    if constexpr (std::same_as<Tag, CountTag>)
        cls.def_rw("arg", &Data::arg);
    else if constexpr (std::same_as<Tag, DistanceTag>)
        cls.def_rw("lhs", &Data::lhs).def_rw("mid", &Data::mid).def_rw("rhs", &Data::rhs);
    else if constexpr (std::same_as<Tag, NumericalConstantTag>)
        cls.def_rw("identifier", &Data::identifier);
    else if constexpr (NumericalBinaryTag<Tag>)
        cls.def_rw("lhs", &Data::lhs).def_rw("rhs", &Data::rhs);
    else
        cls.def_rw("reference", &Data::reference);
}

template<typename Tag>
void bind_numerical_view(nb::module_& m, const char* name)
{
    using Type = Numerical<Tag>;
    using View = urd::index_view<Type, ConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::index);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
    cls.def("syntactic_complexity", [](View view) { return semantics::syntactic_complexity(view); });

    if constexpr (std::same_as<Tag, CountTag>)
        cls.def("get_arg", &View::get_arg);
    else if constexpr (std::same_as<Tag, DistanceTag>)
        cls.def("get_lhs", &View::get_lhs)
            .def("get_mid", &View::get_mid)
            .def("get_rhs", &View::get_rhs);
    else if constexpr (std::same_as<Tag, NumericalConstantTag>)
        cls.def("get_value", &View::get_value);
    else if constexpr (NumericalBinaryTag<Tag>)
        cls.def("get_lhs", &View::get_lhs).def("get_rhs", &View::get_rhs);
    else
        cls.def("get_argument", &View::get_argument);
}

}  // namespace

void bind_semantics_numerical(nb::module_& m)
{
    urd::python::bind_index<urd::index<Numerical<NumericalConstantTag>>>(m, "NumericalConstantIndex");
    bind_numerical_data<NumericalConstantTag>(m, "NumericalConstantData");
    bind_numerical_view<NumericalConstantTag>(m, "NumericalConstant");

    urd::python::bind_index<urd::index<Numerical<AddTag>>>(m, "NumericalAddIndex");
    bind_numerical_data<AddTag>(m, "NumericalAddData");
    bind_numerical_view<AddTag>(m, "NumericalAdd");

    urd::python::bind_index<urd::index<Numerical<SubTag>>>(m, "NumericalSubIndex");
    bind_numerical_data<SubTag>(m, "NumericalSubData");
    bind_numerical_view<SubTag>(m, "NumericalSub");

    urd::python::bind_index<urd::index<Numerical<MulTag>>>(m, "NumericalMulIndex");
    bind_numerical_data<MulTag>(m, "NumericalMulData");
    bind_numerical_view<MulTag>(m, "NumericalMul");

    urd::python::bind_index<urd::index<Numerical<DivTag>>>(m, "NumericalDivIndex");
    bind_numerical_data<DivTag>(m, "NumericalDivData");
    bind_numerical_view<DivTag>(m, "NumericalDiv");

    urd::python::bind_index<urd::index<Numerical<MinTag>>>(m, "NumericalMinIndex");
    bind_numerical_data<MinTag>(m, "NumericalMinData");
    bind_numerical_view<MinTag>(m, "NumericalMin");

    urd::python::bind_index<urd::index<Numerical<MaxTag>>>(m, "NumericalMaxIndex");
    bind_numerical_data<MaxTag>(m, "NumericalMaxData");
    bind_numerical_view<MaxTag>(m, "NumericalMax");

    using Count = Numerical<CountTag>;
    using Distance = Numerical<DistanceTag>;
    using ArgumentReference = Numerical<ArgumentTag<NumericalTag>>;

    urd::python::bind_index<urd::index<Count>>(m, "NumericalCountIndex");
    urd::python::bind_index<urd::index<Distance>>(m, "NumericalDistanceIndex");
    urd::python::bind_index<urd::index<ArgumentReference>>(m, "NumericalArgumentIndex");

    bind_numerical_data<CountTag>(m, "NumericalCountData");
    bind_numerical_data<DistanceTag>(m, "NumericalDistanceData");
    bind_numerical_data<ArgumentTag<NumericalTag>>(m, "NumericalArgumentData");

    bind_numerical_view<CountTag>(m, "NumericalCount");
    bind_numerical_view<DistanceTag>(m, "NumericalDistance");
    bind_numerical_view<ArgumentTag<NumericalTag>>(m, "NumericalArgument");
}

}  // namespace runir::kr::dl
