#ifndef RUNIR_KR_GP_FORMATTER_HPP_
#define RUNIR_KR_GP_FORMATTER_HPP_

#include "runir/common/config.hpp"
#include "runir/kr/gp/condition_view.hpp"
#include "runir/kr/gp/dl/formatter.hpp"
#include "runir/kr/gp/effect_view.hpp"
#include "runir/kr/gp/feature_view.hpp"
#include "runir/kr/gp/policy_executor.hpp"
#include "runir/kr/gp/policy_view.hpp"
#include "runir/kr/gp/rule_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <tyr/common/iostream.hpp>
#include <vector>

namespace runir::kr::gp::format
{

template<typename FeatureTag>
struct NamedFeature
{
    tyr::Index<runir::kr::gp::Feature<FeatureTag>> index;
    std::string name;
};

struct FeatureNames
{
    std::vector<NamedFeature<runir::kr::gp::dl::BooleanFeature>> booleans;
    std::vector<NamedFeature<runir::kr::gp::dl::NumericalFeature>> numericals;
};

template<typename FeatureTag>
auto& named_features(FeatureNames& names)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::gp::dl::BooleanFeature>)
        return names.booleans;
    else if constexpr (std::same_as<FeatureTag, runir::kr::gp::dl::NumericalFeature>)
        return names.numericals;
}

template<typename FeatureTag, typename C>
std::string get_or_create_name(FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::Feature<FeatureTag>>, C> feature)
{
    auto& entries = named_features<FeatureTag>(names);
    for (const auto& entry : entries)
        if (entry.index == feature.get_index())
            return entry.name;

    const auto prefix = std::same_as<FeatureTag, runir::kr::gp::dl::BooleanFeature> ? "b" : "n";
    auto name = fmt::format("{}_{}", prefix, entries.size());
    entries.push_back(NamedFeature<FeatureTag> { feature.get_index(), name });
    return name;
}

template<typename FeatureTag, typename ObservationTag, typename C>
void collect_feature(FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition)
{
    get_or_create_name(names, condition.get_feature());
}

template<typename FeatureTag, typename ObservationTag, typename C>
void collect_feature(FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect)
{
    get_or_create_name(names, effect.get_feature());
}

template<typename C>
void collect_features(FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::ConditionVariant>, C> condition)
{
    condition.get_variant().apply([&](auto concrete_variant)
                                  { concrete_variant.get_variant().apply([&](auto concrete_condition) { collect_feature(names, concrete_condition); }); });
}

template<typename C>
void collect_features(FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::EffectVariant>, C> effect)
{
    effect.get_variant().apply([&](auto concrete_variant)
                               { concrete_variant.get_variant().apply([&](auto concrete_effect) { collect_feature(names, concrete_effect); }); });
}

template<typename C>
FeatureNames collect_features(tyr::View<tyr::Index<runir::kr::gp::Policy>, C> policy)
{
    auto names = FeatureNames {};
    for (auto rule : policy.get_rules())
    {
        for (auto condition : rule.get_conditions())
            collect_features(names, condition);
        for (auto effect : rule.get_effects())
            collect_features(names, effect);
    }
    return names;
}

template<typename FeatureTag, typename C>
std::string feature(tyr::View<tyr::Index<runir::kr::gp::Feature<FeatureTag>>, C> view, std::string_view name)
{
    return view.get_variant().apply([&](auto concrete_feature) { return runir::kr::gp::dl::format::feature(concrete_feature, name); });
}

template<typename FeatureTag, typename C>
std::string feature(tyr::View<tyr::Index<runir::kr::gp::Feature<FeatureTag>>, C> view)
{
    const auto prefix = std::same_as<FeatureTag, runir::kr::gp::dl::BooleanFeature> ? "b" : "n";
    return feature(view, fmt::format("{}_{}", prefix, view.get_index().get_value()));
}

template<typename C>
void append_features(std::ostream& os, const C& context, const FeatureNames& names)
{
    for (const auto& entry : names.booleans)
        os << tyr::print_indent << feature(tyr::make_view(entry.index, context), entry.name) << "\n";
    for (const auto& entry : names.numericals)
        os << tyr::print_indent << feature(tyr::make_view(entry.index, context), entry.name) << "\n";
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string condition(FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return runir::kr::gp::dl::format::condition(view, get_or_create_name(names, view.get_feature()));
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string effect(FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return runir::kr::gp::dl::format::effect(view, get_or_create_name(names, view.get_feature()));
}

template<typename C>
std::string condition(FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::ConditionVariant>, C> view)
{
    return view.get_variant().apply(
        [&](auto concrete_variant)
        { return concrete_variant.get_variant().apply([&](auto concrete_condition) { return condition(names, concrete_condition); }); });
}

template<typename C>
std::string effect(FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::EffectVariant>, C> view)
{
    return view.get_variant().apply([&](auto concrete_variant)
                                    { return concrete_variant.get_variant().apply([&](auto concrete_effect) { return effect(names, concrete_effect); }); });
}

template<typename C>
void append_rule(std::ostream& os, FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::Rule>, C> view)
{
    auto conditions = std::vector<std::string> {};
    for (auto item : view.get_conditions())
        conditions.push_back(condition(names, item));

    auto effects = std::vector<std::string> {};
    for (auto item : view.get_effects())
        effects.push_back(effect(names, item));

    os << tyr::print_indent << "(\n";
    {
        tyr::IndentScope scope(os);
        os << tyr::print_indent << fmt::format("(:conditions {})", fmt::join(conditions, " ")) << "\n";
        os << tyr::print_indent << fmt::format("(:effects {})", fmt::join(effects, " ")) << "\n";
    }
    os << tyr::print_indent << ")";
}

template<typename C>
std::string rule(FeatureNames& names, tyr::View<tyr::Index<runir::kr::gp::Rule>, C> view)
{
    auto os = std::ostringstream {};
    append_rule(os, names, view);
    return os.str();
}

template<typename C>
std::string policy(tyr::View<tyr::Index<runir::kr::gp::Policy>, C> view)
{
    auto names = collect_features(view);
    auto os = std::ostringstream {};

    os << "(policy\n";
    {
        tyr::IndentScope scope(os);

        os << tyr::print_indent << "(:features\n";
        {
            tyr::IndentScope feature_scope(os);
            append_features(os, view.get_context(), names);
        }
        os << tyr::print_indent << ")\n";

        os << tyr::print_indent << "(:rules\n";
        {
            tyr::IndentScope rule_scope(os);
            for (auto item : view.get_rules())
            {
                append_rule(os, names, item);
                os << "\n";
            }
        }
        os << tyr::print_indent << ")\n";
    }
    os << ")\n";

    return os.str();
}

inline std::string policy_proof_status(runir::kr::gp::PolicyProofStatus status)
{
    switch (status)
    {
        case runir::kr::gp::PolicyProofStatus::SUCCESS:
            return "success";
        case runir::kr::gp::PolicyProofStatus::FAILURE:
            return "failure";
    }
    return "unknown";
}

template<tyr::planning::TaskKind Kind>
std::string policy_proof_results(const runir::kr::gp::PolicyProofResults<Kind>& result)
{
    return fmt::format("PolicyProofResults(status={}, graph_vertices={}, graph_edges={}, deadend_transitions={}, open_states={}, cycle={})",
                       policy_proof_status(result.status),
                       result.graph.get_num_vertices(),
                       result.graph.get_num_edges(),
                       result.deadend_transitions.size(),
                       result.open_states.size(),
                       result.cycle.size());
}

}  // namespace runir::kr::gp::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<typename FeatureTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::gp::Feature<FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::gp::Feature<FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::gp::format::feature(view), ctx); }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::gp::ConditionVariant>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::gp::ConditionVariant>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::gp::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::gp::format::condition(names, view), ctx);
    }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::gp::ConcreteConditionVariant<LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::gp::ConcreteConditionVariant<LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::gp::format::FeatureNames {};
        const auto text = view.get_variant().apply([&](auto concrete_condition) { return runir::kr::gp::format::condition(names, concrete_condition); });
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::gp::EffectVariant>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::gp::EffectVariant>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::gp::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::gp::format::effect(names, view), ctx);
    }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::gp::ConcreteEffectVariant<LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::gp::ConcreteEffectVariant<LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::gp::format::FeatureNames {};
        const auto text = view.get_variant().apply([&](auto concrete_effect) { return runir::kr::gp::format::effect(names, concrete_effect); });
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::gp::Rule>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::gp::Rule>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::gp::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::gp::format::rule(names, view), ctx);
    }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::gp::Policy>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::gp::Policy>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::gp::format::policy(view), ctx); }
};

template<>
struct fmt::formatter<runir::kr::gp::PolicyProofStatus> : fmt::formatter<std::string_view>
{
    auto format(runir::kr::gp::PolicyProofStatus status, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::gp::format::policy_proof_status(status), ctx);
    }
};

template<tyr::planning::TaskKind Kind>
struct fmt::formatter<runir::kr::gp::PolicyProofResults<Kind>> : fmt::formatter<std::string_view>
{
    auto format(const runir::kr::gp::PolicyProofResults<Kind>& result, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::gp::format::policy_proof_results(result), ctx);
    }
};
#endif

#endif
