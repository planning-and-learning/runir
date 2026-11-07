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
    auto data = urd::data<kr::ps::MemoryState<kr::ExtFamilyTag>>(name);
    return repository.insert(data).first;
}

auto make_module_data(kr::ps::ext::Repository& repository, const std::string& name) -> urd::data<kr::ps::ext::Module>
{
    auto symbol_data = urd::data<kr::ps::ModuleSymbol<kr::ExtFamilyTag>>(name);
    const auto symbol = repository.insert(symbol_data).first;
    auto data = urd::data<kr::ps::ext::Module>();
    data.symbol = symbol.index();
    return data;
}

auto create_module(kr::ps::ext::Repository& repository,
                   const std::string& name,
                   kr::ps::ext::MemoryStateView entry,
                   std::initializer_list<kr::ps::ext::MemoryStateView> memory_states) -> kr::ps::ext::ModuleView
{
    auto data = make_module_data(repository, name);
    data.entry_memory_state = entry.index();
    for (auto state : memory_states)
        data.memory_states.push_back(state.index());
    kr::ps::ext::canonicalize(data);
    return repository.insert(data).first;
}

auto create_program(kr::ps::ext::Repository& repository, kr::ps::ext::ModuleView entry, std::initializer_list<kr::ps::ext::ModuleView> modules)
    -> kr::ps::ext::ProgramView
{
    auto data = urd::data<kr::ps::ext::Program>();
    data.entry_module = entry.index();
    for (auto module_ : modules)
        data.modules.push_back(module_.index());
    kr::ps::ext::canonicalize(data);
    return repository.insert(data).first;
}

auto create_top_concept(kr::dl::ConstructorRepository& repository) -> kr::dl::ConstructorView<kr::dl::ConceptTag>
{
    auto top_data = urd::data<kr::dl::Concept<kr::dl::SemanticFamily, kr::dl::TopTag>>();
    const auto top = repository.insert(top_data).first;
    auto constructor_data = urd::data<kr::dl::Constructor<kr::dl::ConceptTag>>(top.index());
    return kr::dl::insert(repository, constructor_data).first;
}

}  // namespace runir::tests
