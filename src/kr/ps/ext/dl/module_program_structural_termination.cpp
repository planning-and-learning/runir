#include "runir/graphs/algorithms.hpp"
#include "runir/kr/ps/ext/dl/structural_termination.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "structural_termination/detail.hpp"

#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

namespace runir::kr::ps::ext::dl
{

namespace detail
{

inline void analyze_modules(ModuleProgramView program,
                            ModuleProgramStructuralTerminationResult& result,
                            std::size_t max_features,
                            bool use_incomplete_preprocessing,
                            bool use_memory_scc_scope)
{
    for (auto module : program.get_modules())
    {
        auto module_result = structural_termination(module, max_features, use_incomplete_preprocessing, use_memory_scc_scope);
        if (!module_result.is_terminating())
            result.status = StructuralTerminationStatus::NON_TERMINATING;
        result.module_results.push_back(std::move(module_result));
    }
}

struct ModuleCallGraph
{
    graphs::StaticGraph<std::tuple<>, RuleVariantView> graph;
    std::vector<RuleVariantView> unresolved_rules;
};

inline std::optional<std::size_t> find_module(ModuleProgramView program, ygg::Index<ModuleSymbol> symbol)
{
    const auto modules = program.get_modules();
    for (std::size_t position = 0; position < modules.size(); ++position)
        if (modules[position].get_symbol().get_index() == symbol)
            return position;
    return std::nullopt;
}

inline ModuleCallGraph build_module_call_graph(ModuleProgramView program)
{
    const auto modules = program.get_modules();
    auto builder = graphs::StaticGraphBuilder<std::tuple<>, RuleVariantView> {};
    for (std::size_t position = 0; position < modules.size(); ++position)
        builder.add_vertex();

    auto unresolved_rules = std::vector<RuleVariantView> {};
    for (std::size_t source_module = 0; source_module < modules.size(); ++source_module)
    {
        for (auto transition : modules[source_module].get_memory_transitions())
        {
            for (auto rule_variant : transition)
            {
                ygg::visit(
                    [&](auto rule)
                    {
                        using RuleViewT = std::decay_t<decltype(rule)>;
                        if constexpr (std::same_as<RuleViewT, ygg::View<ygg::Index<Rule<CallTag>>, Repository>>)
                        {
                            const auto target_module = find_module(program, rule.get_callee().get_index());
                            if (!target_module)
                            {
                                unresolved_rules.push_back(rule_variant);
                                return;
                            }
                            builder.add_directed_edge(static_cast<graphs::VertexIndex>(source_module),
                                                      static_cast<graphs::VertexIndex>(*target_module),
                                                      rule_variant);
                        }
                    },
                    rule_variant.get_variant());
            }
        }
    }

    return { graphs::StaticGraph<std::tuple<>, RuleVariantView> { std::move(builder) }, std::move(unresolved_rules) };
}

std::vector<RuleVariantView> find_recursive_call_rules(ModuleProgramView program)
{
    const auto call_graph = build_module_call_graph(program);
    auto result = call_graph.unresolved_rules;
    const auto [num_components, component_of] = graphs::algorithms::strong_components(call_graph.graph);
    static_cast<void>(num_components);
    for (const auto& edge : call_graph.graph.get_edges())
        if (component_of[edge.get_source()] == component_of[edge.get_target()])
            result.push_back(edge.get_property());
    return result;
}

}  // namespace detail

ModuleProgramStructuralTerminationResult
structural_termination(ModuleProgramView program, std::size_t max_features, bool use_incomplete_preprocessing, bool use_memory_scc_scope)
{
    auto result = ModuleProgramStructuralTerminationResult {};
    detail::analyze_modules(program, result, max_features, use_incomplete_preprocessing, use_memory_scc_scope);
    result.recursive_call_rules = detail::find_recursive_call_rules(program);
    if (!result.recursive_call_rules.empty())
        result.status = StructuralTerminationStatus::NON_TERMINATING;
    return result;
}

}  // namespace runir::kr::ps::ext::dl
