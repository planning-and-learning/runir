#ifndef PYRUNIR_KR_DL_BINDING_HELPERS_HPP_
#define PYRUNIR_KR_DL_BINDING_HELPERS_HPP_

#include <cctype>
#include <fmt/format.h>
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <runir/knowledge_representation/dl/declarations.hpp>
#include <string>
#include <string_view>
#include <tyr/common/equal_to.hpp>
#include <tyr/common/hash.hpp>
#include <tyr/common/type_list.hpp>
#include <tyr/common/types.hpp>

namespace runir::kr::dl
{

namespace nb = nanobind;

namespace python_detail
{

template<typename List, typename F>
struct ForEachType;

template<typename... Ts, typename F>
struct ForEachType<tyr::TypeList<Ts...>, F>
{
    static void apply(F&& f) { (f.template operator()<Ts>(), ...); }
};

template<typename List, typename F>
void for_each_type(F&& f)
{
    ForEachType<List, F>::apply(std::forward<F>(f));
}

template<typename T>
auto to_string(const T& value)
{
    return fmt::format("{}", value);
}

template<typename T>
void bind_index(nb::module_& m, const std::string& name)
{
    nb::class_<T>(m, name.c_str())
        .def(nb::init<>())
        .def(nb::init<tyr::uint_t>(), nb::arg("index"))
        .def("__int__", [](const T& index) { return tyr::uint_t(index); })
        .def("__index__", [](const T& index) { return tyr::uint_t(index); })
        .def("__hash__", [](const T& index) { return tyr::uint_t(index); })
        .def("__eq__", [](const T& lhs, const T& rhs) { return lhs == rhs; })
        .def("__lt__", [](const T& lhs, const T& rhs) { return lhs < rhs; })
        .def("__le__", [](const T& lhs, const T& rhs) { return lhs <= rhs; })
        .def("__gt__", [](const T& lhs, const T& rhs) { return lhs > rhs; })
        .def("__ge__", [](const T& lhs, const T& rhs) { return lhs >= rhs; })
        .def("__repr__", [name](const T& index) { return name + "(" + std::to_string(tyr::uint_t(index)) + ")"; });
}

template<typename T>
void bind_data(nb::module_& m, const std::string& name)
{
    using Data = tyr::Data<T>;

    auto cls = nb::class_<Data>(m, name.c_str()).def(nb::init<>());

    if constexpr (requires { &Data::index; })
        cls.def_rw("index", &Data::index);
    if constexpr (requires { &Data::arg; })
        cls.def_rw("arg", &Data::arg);
    if constexpr (requires { &Data::lhs; })
        cls.def_rw("lhs", &Data::lhs);
    if constexpr (requires { &Data::mid; })
        cls.def_rw("mid", &Data::mid);
    if constexpr (requires { &Data::rhs; })
        cls.def_rw("rhs", &Data::rhs);
    if constexpr (requires { &Data::predicate; })
        cls.def_rw("predicate", &Data::predicate);
    if constexpr (requires { &Data::polarity; })
        cls.def_rw("polarity", &Data::polarity);
    if constexpr (requires { &Data::object; })
        cls.def_rw("object", &Data::object);
}

template<typename T, typename Repository>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = tyr::View<tyr::Index<T>, Repository>;

    auto cls = nb::class_<View>(m, name.c_str())
                   .def("__str__", [](const View& self) { return to_string(self); })
                   .def("__repr__", [](const View& self) { return to_string(self); })
                   .def("get_index", &View::get_index);

    if constexpr (requires(const View& view) { view.get_arg(); })
        cls.def("get_arg", &View::get_arg);
    if constexpr (requires(const View& view) { view.get_lhs(); })
        cls.def("get_lhs", &View::get_lhs);
    if constexpr (requires(const View& view) { view.get_mid(); })
        cls.def("get_mid", &View::get_mid);
    if constexpr (requires(const View& view) { view.get_rhs(); })
        cls.def("get_rhs", &View::get_rhs);
    if constexpr (requires(const View& view) { view.get_predicate(); })
        cls.def("get_predicate", &View::get_predicate);
    if constexpr (requires(const View& view) { view.get_polarity(); })
        cls.def("get_polarity", &View::get_polarity);
    if constexpr (requires(const View& view) { view.get_object(); })
        cls.def("get_object", &View::get_object);
}

inline std::string pascal_case(std::string_view value)
{
    auto result = std::string();
    auto capitalize = true;

    for (auto c : value)
    {
        if (c == '_')
        {
            capitalize = true;
            continue;
        }

        result.push_back(capitalize ? static_cast<char>(std::toupper(static_cast<unsigned char>(c))) : c);
        capitalize = false;
    }

    return result;
}

template<CategoryTag Category>
std::string category_name()
{
    return pascal_case(Category::name);
}

template<typename T>
std::string fact_kind_name_value()
{
    if constexpr (requires { T::name; })
        return T::name;
    else if constexpr (std::same_as<T, tyr::formalism::StaticTag>)
        return "Static";
    else if constexpr (std::same_as<T, tyr::formalism::FluentTag>)
        return "Fluent";
    else if constexpr (std::same_as<T, tyr::formalism::DerivedTag>)
        return "Derived";
    else if constexpr (std::same_as<T, tyr::formalism::AuxiliaryTag>)
        return "Auxiliary";
    else
        static_assert(tyr::dependent_false<T>::value);
}

template<typename Tag>
std::string fact_kind_name()
{
    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        return fact_kind_name_value<typename Tag::FactKind>();
    else
        return "";
}

template<typename Category, typename Tag>
std::string concrete_name()
{
    return pascal_case(std::string(Category::name) + "_" + Tag::name) + fact_kind_name<Tag>();
}

template<template<typename> typename Element, typename Category>
struct ConcreteIndexBinder
{
    nb::module_& m;

    template<typename Tag>
    void operator()()
    {
        bind_index<tyr::Index<Element<Tag>>>(m, concrete_name<Category, Tag>() + "Index");
    }
};

template<template<typename> typename Element, typename Category>
struct ConcreteDataBinder
{
    nb::module_& m;

    template<typename Tag>
    void operator()()
    {
        bind_data<Element<Tag>>(m, concrete_name<Category, Tag>() + "Data");
    }
};

template<template<typename> typename Element, typename Category, typename Repository>
struct ConcreteViewBinder
{
    nb::module_& m;

    template<typename Tag>
    void operator()()
    {
        bind_view<Element<Tag>, Repository>(m, concrete_name<Category, Tag>());
    }
};

template<template<typename> typename Element, typename Category, typename Tags>
void bind_concrete_indices(nb::module_& m)
{
    for_each_type<Tags>(ConcreteIndexBinder<Element, Category> { m });
}

template<template<typename> typename Element, typename Category, typename Tags>
void bind_concrete_datas(nb::module_& m)
{
    for_each_type<Tags>(ConcreteDataBinder<Element, Category> { m });
}

template<template<typename> typename Element, typename Category, typename Tags, typename Repository>
void bind_concrete_views(nb::module_& m)
{
    for_each_type<Tags>(ConcreteViewBinder<Element, Category, Repository> { m });
}

template<template<typename> typename Constructor, typename Category>
void bind_constructor_index(nb::module_& m)
{
    bind_index<tyr::Index<Constructor<Category>>>(m, category_name<Category>() + "Index");
}

template<template<typename> typename Constructor, typename Category>
void bind_constructor_data(nb::module_& m)
{
    bind_data<Constructor<Category>>(m, category_name<Category>() + "Data");
}

template<template<typename> typename Constructor, typename Category, typename Repository>
void bind_constructor_view(nb::module_& m)
{
    bind_view<Constructor<Category>, Repository>(m, category_name<Category>());
}

template<template<typename> typename Concept,
         template<typename>
         typename Role,
         template<typename>
         typename Boolean,
         template<typename>
         typename Numerical,
         template<typename>
         typename Constructor>
void bind_constructor_indices(nb::module_& m)
{
    bind_concrete_indices<Concept, ConceptTag, ConceptConstructorTags>(m);
    bind_concrete_indices<Role, RoleTag, RoleConstructorTags>(m);
    bind_concrete_indices<Boolean, BooleanTag, BooleanConstructorTags>(m);
    bind_concrete_indices<Numerical, NumericalTag, NumericalConstructorTags>(m);
    bind_constructor_index<Constructor, ConceptTag>(m);
    bind_constructor_index<Constructor, RoleTag>(m);
    bind_constructor_index<Constructor, BooleanTag>(m);
    bind_constructor_index<Constructor, NumericalTag>(m);
}

template<template<typename> typename Concept,
         template<typename>
         typename Role,
         template<typename>
         typename Boolean,
         template<typename>
         typename Numerical,
         template<typename>
         typename Constructor>
void bind_constructor_datas(nb::module_& m)
{
    bind_concrete_datas<Concept, ConceptTag, ConceptConstructorTags>(m);
    bind_concrete_datas<Role, RoleTag, RoleConstructorTags>(m);
    bind_concrete_datas<Boolean, BooleanTag, BooleanConstructorTags>(m);
    bind_concrete_datas<Numerical, NumericalTag, NumericalConstructorTags>(m);
    bind_constructor_data<Constructor, ConceptTag>(m);
    bind_constructor_data<Constructor, RoleTag>(m);
    bind_constructor_data<Constructor, BooleanTag>(m);
    bind_constructor_data<Constructor, NumericalTag>(m);
}

template<template<typename> typename Concept,
         template<typename>
         typename Role,
         template<typename>
         typename Boolean,
         template<typename>
         typename Numerical,
         template<typename>
         typename Constructor,
         typename Repository>
void bind_constructor_views(nb::module_& m)
{
    bind_constructor_view<Constructor, ConceptTag, Repository>(m);
    bind_constructor_view<Constructor, RoleTag, Repository>(m);
    bind_constructor_view<Constructor, BooleanTag, Repository>(m);
    bind_constructor_view<Constructor, NumericalTag, Repository>(m);
    bind_concrete_views<Concept, ConceptTag, ConceptConstructorTags, Repository>(m);
    bind_concrete_views<Role, RoleTag, RoleConstructorTags, Repository>(m);
    bind_concrete_views<Boolean, BooleanTag, BooleanConstructorTags, Repository>(m);
    bind_concrete_views<Numerical, NumericalTag, NumericalConstructorTags, Repository>(m);
}

template<typename T>
void bind_clearable_repository(nb::class_<T>& cls)
{
    cls.def("clear", &T::clear).def("get_index", &T::get_index);
}

}  // namespace python_detail

}  // namespace runir::kr::dl

#endif
