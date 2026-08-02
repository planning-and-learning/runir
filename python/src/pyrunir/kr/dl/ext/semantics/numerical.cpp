#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/numerical_view.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::ext
{
namespace
{

template<typename Tag>
void bind_numerical_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<Numerical<runir::kr::ExtFamilyTag, Tag>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(cls);

    if constexpr (std::same_as<Tag, CountTag>)
        cls.def_rw("arg", &Data::arg);
    else if constexpr (std::same_as<Tag, DistanceTag>)
        cls.def_rw("lhs", &Data::lhs).def_rw("mid", &Data::mid).def_rw("rhs", &Data::rhs);
    else
        cls.def_rw("reference", &Data::reference);
}

template<typename Tag>
void bind_numerical_view(nb::module_& m, const char* name)
{
    using Type = Numerical<runir::kr::ExtFamilyTag, Tag>;
    using View = ygg::View<ygg::Index<Type>, ExtConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    cls.def("syntactic_complexity", [](View view) { return semantics::syntactic_complexity(view); });

    if constexpr (std::same_as<Tag, CountTag>)
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
    else if constexpr (std::same_as<Tag, DistanceTag>)
        cls.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>())
            .def("get_mid", &View::get_mid, nb::keep_alive<0, 1>())
            .def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
    else
        cls.def("get_argument", &View::get_argument, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_semantics_numerical(nb::module_& m)
{
    using Count = Numerical<runir::kr::ExtFamilyTag, CountTag>;
    using Distance = Numerical<runir::kr::ExtFamilyTag, DistanceTag>;
    using ArgumentReference = Numerical<runir::kr::ExtFamilyTag, ArgumentTag<NumericalTag>>;

    ygg::bind_index<ygg::Index<Count>>(m, "NumericalCountIndex");
    ygg::bind_index<ygg::Index<Distance>>(m, "NumericalDistanceIndex");
    ygg::bind_index<ygg::Index<ArgumentReference>>(m, "NumericalArgumentIndex");

    bind_numerical_data<CountTag>(m, "NumericalCountData");
    bind_numerical_data<DistanceTag>(m, "NumericalDistanceData");
    bind_numerical_data<ArgumentTag<NumericalTag>>(m, "NumericalArgumentData");

    bind_numerical_view<CountTag>(m, "NumericalCount");
    bind_numerical_view<DistanceTag>(m, "NumericalDistance");
    bind_numerical_view<ArgumentTag<NumericalTag>>(m, "NumericalArgument");
}

}  // namespace runir::kr::dl::ext
