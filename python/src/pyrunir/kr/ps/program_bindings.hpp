#ifndef PYRUNIR_KR_PS_PROGRAM_BINDINGS_HPP_
#define PYRUNIR_KR_PS_PROGRAM_BINDINGS_HPP_

#include "pyrunir/conversions.hpp"
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
#include <urd/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

/// Bindings every policy family (base, ext, icp) or program family (ext, icp) shares; each family's module calls them
/// with its own repository binding.
namespace runir::kr::python
{

namespace nb = nanobind;

template<runir::kr::FamilyTag Family, typename RepositoryTypes>
void bind_repository(nb::module_& /*m*/, nb::class_<runir::kr::ps::BasicRepository<Family, RepositoryTypes>>& repository)
{
    using namespace nb::literals;
    using Repository = runir::kr::ps::BasicRepository<Family, RepositoryTypes>;
    repository
        .def(nb::new_([](runir::kr::dl::ConstructorRepositoryPtr dl_repository) { return std::make_shared<Repository>(std::move(dl_repository)); }),
             "dl_repository"_a)
        .def("clear", &Repository::clear)
        .def("layer", &Repository::layer)
        .def(
            "get_dl_repository",
            [](Repository& self) -> auto& { return self.template get<runir::kr::dl::ConstructorRepository>(); },
            nb::rv_policy::reference_internal);
}

/// The views of the memory states, module symbols and rule variants of a program family (ext, icp), whose records are
/// bound first (bind_data), and their insertion into the family's repository.
template<runir::kr::ps::ProgramFamilyTag Family>
void bind_program_record_views(nb::module_& m, nb::class_<runir::kr::ps::Repository<Family>>& repository)
{
    using Repository = runir::kr::ps::Repository<Family>;
    urd::python::bind_view<urd::index_view<runir::kr::ps::MemoryState<Family>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::MemoryState<Family>, Repository>(m);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ModuleSymbol<Family>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ModuleSymbol<Family>, Repository>(m);
    urd::python::bind_view<urd::index_view<runir::kr::ps::Rule<Family>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::Rule<Family>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::MemoryState<Family>>(repository);
    ygg::bind_insert<runir::kr::ps::ModuleSymbol<Family>>(repository);
    ygg::bind_insert<runir::kr::ps::Rule<Family>>(repository);
}

}  // namespace runir::kr::python

#endif
