#include "module_fixtures.hpp"

#include "fixtures.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <yggdrasil/serialization/json.hpp>

namespace runir::tests
{

auto formatter_fragment(std::string_view name) -> std::string
{
    static const auto fixture = load_fixture_json("kr/ps/ext/formatter_fragments.json");
    return ygg::common::as_string(ygg::common::as_object(fixture, "formatter_fragments"), name, "formatter_fragments");
}

auto create_memory_state(kr::ps::ext::Repository& repository, const std::string& name) -> kr::ps::ext::MemoryStateView
{
    auto data = ygg::Data<kr::ps::ext::MemoryState>(name);
    return repository.get_or_create(data).first;
}

auto make_module_data(kr::ps::ext::Repository& repository, const std::string& name) -> ygg::Data<kr::ps::ext::Module>
{
    auto symbol_data = ygg::Data<kr::ps::ext::ModuleSymbol>(name);
    const auto symbol = repository.get_or_create(symbol_data).first;
    return ygg::Data<kr::ps::ext::Module>(symbol.get_index());
}

auto create_module(kr::ps::ext::Repository& repository,
                   const std::string& name,
                   kr::ps::ext::MemoryStateView entry,
                   std::initializer_list<kr::ps::ext::MemoryStateView> memory_states) -> kr::ps::ext::ModuleView
{
    auto data = make_module_data(repository, name);
    data.entry_memory_state = entry.get_index();
    for (auto state : memory_states)
        data.memory_states.push_back(state.get_index());
    kr::ps::ext::canonicalize(data);
    return repository.get_or_create(data).first;
}

auto create_module_program(kr::ps::ext::Repository& repository,
                           kr::ps::ext::ModuleView entry,
                           std::initializer_list<kr::ps::ext::ModuleView> modules) -> kr::ps::ext::ModuleProgramView
{
    auto data = ygg::Data<kr::ps::ext::ModuleProgram>();
    data.entry_module = entry.get_index();
    for (auto module : modules)
        data.modules.push_back(module.get_index());
    kr::ps::ext::canonicalize(data);
    return repository.get_or_create(data).first;
}

auto create_top_concept(kr::dl::ExtConstructorRepository& repository) -> kr::dl::ExtConstructorView<kr::dl::ConceptTag>
{
    auto top_data = ygg::Data<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::TopTag>>();
    const auto top = repository.get_or_create(top_data).first;
    auto constructor_data = ygg::Data<kr::dl::Constructor<kr::ExtFamilyTag, kr::dl::ConceptTag>>(top.get_index());
    return repository.get_or_create(constructor_data).first;
}

}  // namespace runir::tests
