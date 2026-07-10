#include "runir/graphs/algorithms.hpp"
#include "runir/kr/ps/ext/dl/structural_termination.hpp"
#include "structural_termination/detail.hpp"

#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace runir::kr::ps::ext::dl
{

namespace detail
{

inline std::vector<ModuleView> analyze_modules(ModuleProgramView program, ModuleProgramStructuralTerminationResult& result)
{
    auto modules = std::vector<ModuleView> {};
    for (auto module : program.get_modules())
    {
        modules.push_back(module);
        auto module_result = structural_termination(module);
        if (!module_result.is_terminating())
            result.status = StructuralTerminationStatus::NON_TERMINATING;
        result.module_results.push_back(std::move(module_result));
    }
    return modules;
}

struct ModuleCallEdge
{
    std::size_t source_module;
    std::size_t target_module;
    RuleVariantView rule;
};

struct ModuleCallGraph
{
    graphs::StaticGraph<> graph;
    std::vector<ModuleCallEdge> edges;
    std::vector<RuleVariantView> unresolved_rules;
};

inline std::optional<std::size_t> find_module(const std::vector<ModuleView>& modules, const std::string& name)
{
    for (std::size_t position = 0; position < modules.size(); ++position)
        if (std::string(modules[position].get_name()) == name)
            return position;
    return std::nullopt;
}

inline ModuleCallGraph build_module_call_graph(const std::vector<ModuleView>& modules)
{
    auto builder = graphs::StaticGraphBuilder<> {};
    for (std::size_t position = 0; position < modules.size(); ++position)
        builder.add_vertex();

    auto edges = std::vector<ModuleCallEdge> {};
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
                            const auto target_module = find_module(modules, rule.get_callee_name());
                            if (!target_module)
                            {
                                unresolved_rules.push_back(rule_variant);
                                return;
                            }
                            builder.add_directed_edge(static_cast<graphs::VertexIndex>(source_module), static_cast<graphs::VertexIndex>(*target_module));
                            edges.push_back(ModuleCallEdge { source_module, *target_module, rule_variant });
                        }
                    },
                    rule_variant.get_variant());
            }
        }
    }

    return { graphs::StaticGraph<> { std::move(builder) }, std::move(edges), std::move(unresolved_rules) };
}

inline void record_recursive_calls(const ModuleCallGraph& call_graph, ModuleProgramStructuralTerminationResult& result)
{
    for (auto rule : call_graph.unresolved_rules)
    {
        result.status = StructuralTerminationStatus::NON_TERMINATING;
        result.recursive_call_rules.push_back(rule);
    }

    const auto [num_components, component_of] = graphs::algorithms::strong_components(call_graph.graph);
    static_cast<void>(num_components);
    for (const auto& edge : call_graph.edges)
    {
        if (component_of[edge.source_module] == component_of[edge.target_module])
        {
            result.status = StructuralTerminationStatus::NON_TERMINATING;
            result.recursive_call_rules.push_back(edge.rule);
        }
    }
}

}  // namespace detail

ModuleProgramStructuralTerminationResult structural_termination(ModuleProgramView program)
{
    auto result = ModuleProgramStructuralTerminationResult {};
    const auto modules = detail::analyze_modules(program, result);
    const auto call_graph = detail::build_module_call_graph(modules);
    detail::record_recursive_calls(call_graph, result);
    return result;
}

}  // namespace runir::kr::ps::ext::dl
