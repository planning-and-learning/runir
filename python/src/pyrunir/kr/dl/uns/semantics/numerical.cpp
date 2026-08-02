#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/numerical_view.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::uns
{
namespace
{

template<typename Tag>
void bind_numerical_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, Tag>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(cls);

    if constexpr (std::same_as<Tag, CountTag>)
        cls.def_rw("arg", &Data::arg);
    else if constexpr (std::same_as<Tag, DistanceTag>)
        cls.def_rw("lhs", &Data::lhs).def_rw("mid", &Data::mid).def_rw("rhs", &Data::rhs);
    else if constexpr (std::same_as<Tag, NumericalConstantTag>)
        cls.def_rw("identifier", &Data::identifier);
    else
        cls.def_rw("lhs", &Data::lhs).def_rw("rhs", &Data::rhs);
}

template<typename Tag>
void bind_numerical_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, Tag>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::UnsConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    cls.def("syntactic_complexity", [](View view) { return runir::kr::dl::semantics::syntactic_complexity(view); });

    if constexpr (std::same_as<Tag, CountTag>)
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
    else if constexpr (std::same_as<Tag, DistanceTag>)
        cls.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>())
            .def("get_mid", &View::get_mid, nb::keep_alive<0, 1>())
            .def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
    else if constexpr (std::same_as<Tag, NumericalConstantTag>)
        cls.def("get_value", &View::get_value);
    else
        cls.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>()).def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_semantics_numerical(nb::module_& m)
{
    using Count = runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, CountTag>;
    using Distance = runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, DistanceTag>;
    using Constant = runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, NumericalConstantTag>;
    using Add = runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, AddTag>;
    using Sub = runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, SubTag>;
    using Mul = runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, MulTag>;
    using Div = runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, DivTag>;
    using Min = runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, MinTag>;
    using Max = runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, MaxTag>;

    ygg::bind_index<ygg::Index<Count>>(m, "NumericalCountIndex");
    ygg::bind_index<ygg::Index<Distance>>(m, "NumericalDistanceIndex");
    ygg::bind_index<ygg::Index<Constant>>(m, "NumericalConstantIndex");
    ygg::bind_index<ygg::Index<Add>>(m, "NumericalAddIndex");
    ygg::bind_index<ygg::Index<Sub>>(m, "NumericalSubIndex");
    ygg::bind_index<ygg::Index<Mul>>(m, "NumericalMulIndex");
    ygg::bind_index<ygg::Index<Div>>(m, "NumericalDivIndex");
    ygg::bind_index<ygg::Index<Min>>(m, "NumericalMinIndex");
    ygg::bind_index<ygg::Index<Max>>(m, "NumericalMaxIndex");

    bind_numerical_data<CountTag>(m, "NumericalCountData");
    bind_numerical_data<DistanceTag>(m, "NumericalDistanceData");
    bind_numerical_data<NumericalConstantTag>(m, "NumericalConstantData");
    bind_numerical_data<AddTag>(m, "NumericalAddData");
    bind_numerical_data<SubTag>(m, "NumericalSubData");
    bind_numerical_data<MulTag>(m, "NumericalMulData");
    bind_numerical_data<DivTag>(m, "NumericalDivData");
    bind_numerical_data<MinTag>(m, "NumericalMinData");
    bind_numerical_data<MaxTag>(m, "NumericalMaxData");

    bind_numerical_view<CountTag>(m, "NumericalCount");
    bind_numerical_view<DistanceTag>(m, "NumericalDistance");
    bind_numerical_view<NumericalConstantTag>(m, "NumericalConstant");
    bind_numerical_view<AddTag>(m, "NumericalAdd");
    bind_numerical_view<SubTag>(m, "NumericalSub");
    bind_numerical_view<MulTag>(m, "NumericalMul");
    bind_numerical_view<DivTag>(m, "NumericalDiv");
    bind_numerical_view<MinTag>(m, "NumericalMin");
    bind_numerical_view<MaxTag>(m, "NumericalMax");
}

}  // namespace runir::kr::dl::uns
