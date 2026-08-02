#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/numerical_view.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::base
{
namespace
{

template<typename Tag>
void bind_numerical_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, Tag>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(cls);

    if constexpr (std::same_as<Tag, CountTag>)
        cls.def_rw("arg", &Data::arg);
    else
        cls.def_rw("lhs", &Data::lhs).def_rw("mid", &Data::mid).def_rw("rhs", &Data::rhs);
}

template<typename Tag>
void bind_numerical_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, Tag>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::BaseConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    cls.def("syntactic_complexity", [](View view) { return runir::kr::dl::semantics::syntactic_complexity(view); });

    if constexpr (std::same_as<Tag, CountTag>)
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
    else
        cls.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>())
            .def("get_mid", &View::get_mid, nb::keep_alive<0, 1>())
            .def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_semantics_numerical(nb::module_& m)
{
    using Count = runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, CountTag>;
    using Distance = runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, DistanceTag>;

    ygg::bind_index<ygg::Index<Count>>(m, "NumericalCountIndex");
    ygg::bind_index<ygg::Index<Distance>>(m, "NumericalDistanceIndex");
    bind_numerical_data<CountTag>(m, "NumericalCountData");
    bind_numerical_data<DistanceTag>(m, "NumericalDistanceData");
    bind_numerical_view<CountTag>(m, "NumericalCount");
    bind_numerical_view<DistanceTag>(m, "NumericalDistance");
}

}  // namespace runir::kr::dl::base
