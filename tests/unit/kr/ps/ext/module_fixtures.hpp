#ifndef RUNIR_TESTS_UNIT_KR_PS_EXT_MODULE_FIXTURES_HPP_
#define RUNIR_TESTS_UNIT_KR_PS_EXT_MODULE_FIXTURES_HPP_

#include <initializer_list>
#include <runir/kr/ps/ext/declarations.hpp>
#include <string>
#include <string_view>

namespace runir::tests
{

auto formatter_fragment(std::string_view name) -> std::string;

auto create_memory_state(kr::ps::ext::Repository& repository, const std::string& name) -> kr::ps::ext::MemoryStateView;

auto make_module_data(kr::ps::ext::Repository& repository, const std::string& name) -> ygg::Data<kr::ps::ext::Module>;

auto create_module(kr::ps::ext::Repository& repository,
                   const std::string& name,
                   kr::ps::ext::MemoryStateView entry,
                   std::initializer_list<kr::ps::ext::MemoryStateView> memory_states) -> kr::ps::ext::ModuleView;

auto create_module_program(kr::ps::ext::Repository& repository,
                           kr::ps::ext::ModuleView entry,
                           std::initializer_list<kr::ps::ext::ModuleView> modules) -> kr::ps::ext::ModuleProgramView;

auto create_top_concept(kr::dl::ExtConstructorRepository& repository) -> kr::dl::ExtConstructorView<kr::dl::ConceptTag>;

}  // namespace runir::tests

#endif
