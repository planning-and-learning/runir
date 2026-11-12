#ifndef PYRUNIR_KR_PS_PROGRAM_BINDINGS_HPP_
#define PYRUNIR_KR_PS_PROGRAM_BINDINGS_HPP_

#include "pyrunir/kr/binding_utils.hpp"
#include "pyrunir/module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/variant.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/formatter.hpp>
#include <runir/kr/ps/memory_state_data.hpp>
#include <runir/kr/ps/memory_state_view.hpp>
#include <runir/kr/ps/module_symbol_data.hpp>
#include <runir/kr/ps/module_symbol_view.hpp>
#include <runir/kr/ps/repository.hpp>
#include <runir/kr/ps/rule_variant_data.hpp>
#include <runir/kr/ps/rule_variant_view.hpp>
#include <string>
#include <yggdrasil/python/bindings.hpp>

/// Bindings every policy family (base, ext, icp) or program family (ext, icp) shares; each family's module calls them
/// with its own repository binding.
namespace runir::kr::python
{

namespace nb = nanobind;

template<runir::kr::FamilyTag Family, typename RepositoryTypes>
void bind_repository(nb::module_& m, nb::class_<runir::kr::ps::BasicRepository<Family, RepositoryTypes>>& repository)
{
    using namespace nb::literals;
    using Repository = runir::kr::ps::BasicRepository<Family, RepositoryTypes>;
    using RepositoryFactory = runir::kr::ps::BasicRepositoryFactory<Family, RepositoryTypes>;
    repository.def("clear", &Repository::clear)
        .def("layer", &Repository::layer)
        .def("get_dl_repository", [](Repository& self) -> auto& { return self.get_dl_repository(); }, nb::rv_policy::reference_internal);

    auto factory = nb::class_<RepositoryFactory>(m, "RepositoryFactory");
    factory.def(nb::init<>()).def("create", &RepositoryFactory::create, "dl_repository"_a);
}

/// A memory state or module symbol: a record holding its name.
template<typename T, typename Repository>
void bind_named(nb::module_& m, nb::class_<Repository>& repository, const std::string& name)
{
    using Data = urd::data<T>;
    using View = urd::index_view<T, Repository>;
    urd::python::bind_index<urd::index<T>>(m, name + "Index");
    auto data = nb::class_<Data>(m, (name + "Data").c_str())
                    .def(nb::init<>())
                    .def(nb::init<urd::flat_string>(), nb::arg("name"))
                    .def_rw("index", &Data::index)
                    .def_rw("name", &Data::name);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, name.c_str()).def("index", &View::index).def("get_name", &View::get_name);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    ygg::bind_insert<T>(repository);
}

template<runir::kr::ps::ProgramFamilyTag Family>
void bind_rule_variant(nb::module_& m, nb::class_<runir::kr::ps::Repository<Family>>& repository)
{
    using T = runir::kr::ps::Rule<Family>;
    using Repository = runir::kr::ps::Repository<Family>;
    using Data = urd::data<T>;
    using View = urd::index_view<T, Repository>;
    urd::python::bind_index<urd::index<T>>(m, "RuleVariantIndex");
    auto data =
        nb::class_<Data>(m, "RuleVariantData")
            .def(nb::init<>())
            .def(nb::init<urd::flat_string, typename Data::Variant>(), nb::arg("symbol"), nb::arg("variant"))
            .def(urd::python::init_from_views<urd::flat_string, urd::resolved_t<typename Data::Variant, Repository>>(), nb::arg("symbol"), nb::arg("variant"))
            .def_rw("index", &Data::index)
            .def_rw("symbol", &Data::symbol)
            .def_rw("variant", &Data::variant);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, "RuleVariant").def("index", &View::index).def("get_symbol", &View::get_symbol).def("get_variant", &View::get_variant);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    ygg::bind_insert<T>(repository);
}

}  // namespace runir::kr::python

#endif
