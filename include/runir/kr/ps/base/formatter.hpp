#ifndef RUNIR_KR_PS_BASE_FORMATTER_HPP_
#define RUNIR_KR_PS_BASE_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/datasets/formatter.hpp"
#include "runir/kr/ps/base/dl/formatter.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_executor.hpp"
#include "runir/kr/ps/base/sketch_proof_graph.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::base::format
{

template<typename FeatureTag>
struct NamedFeature
{
    ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> index;
    std::string name;
};

struct FeatureNames
{
    std::vector<NamedFeature<runir::kr::ps::dl::BooleanFeature>> booleans;
    std::vector<NamedFeature<runir::kr::ps::dl::NumericalFeature>> numericals;
};

template<typename FeatureTag>
auto& named_features(FeatureNames& names)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        return names.booleans;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
        return names.numericals;
}

template<typename FeatureTag, typename C>
std::string feature_symbol(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>, C> feature)
{
    return ygg::visit([](auto concrete_feature) { return std::string(concrete_feature.get_symbol()); }, feature.get_variant());
}

template<typename FeatureTag, typename C>
std::string get_or_create_name(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>, C> feature)
{
    auto& entries = named_features<FeatureTag>(names);
    for (const auto& entry : entries)
        if (entry.index == feature.get_index())
            return entry.name;

    auto name = feature_symbol(feature);
    entries.push_back(NamedFeature<FeatureTag> { feature.get_index(), name });
    return name;
}

template<typename FeatureTag, typename C>
void append_declared_feature(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>, C> feature)
{
    get_or_create_name(names, feature);
}

template<typename C>
FeatureNames declared_features(ygg::View<ygg::Index<runir::kr::ps::base::Sketch>, C> sketch)
{
    auto names = FeatureNames {};
    for (auto feature : sketch.template get_features<runir::kr::ps::dl::BooleanFeature>())
        append_declared_feature(names, feature);
    for (auto feature : sketch.template get_features<runir::kr::ps::dl::NumericalFeature>())
        append_declared_feature(names, feature);
    return names;
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>, C> view, std::string_view name)
{
    return ygg::visit([&](auto concrete_feature) { return runir::kr::ps::base::dl::format::feature(concrete_feature, name); }, view.get_variant());
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>, C> view)
{
    const auto prefix = std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> ? "b" : "n";
    return feature(view, fmt::format("{}_{}", prefix, view.get_index().get_value()));
}

template<typename C>
void append_features(std::ostream& os, const C& context, const FeatureNames& names)
{
    for (const auto& entry : names.booleans)
    {
        os << ygg::print_indent;
        ygg::visit([&](auto concrete_feature) { runir::kr::ps::base::dl::format::append_feature(os, concrete_feature); },
                   ygg::make_view(entry.index, context).get_variant());
        os << "\n";
    }
    for (const auto& entry : names.numericals)
    {
        os << ygg::print_indent;
        ygg::visit([&](auto concrete_feature) { runir::kr::ps::base::dl::format::append_feature(os, concrete_feature); },
                   ygg::make_view(entry.index, context).get_variant());
        os << "\n";
    }
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string condition(FeatureNames& names,
                      ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return runir::kr::ps::base::dl::format::condition(view, get_or_create_name(names, view.get_feature()));
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string effect(FeatureNames& names,
                   ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return runir::kr::ps::base::dl::format::effect(view, get_or_create_name(names, view.get_feature()));
}

template<typename C>
std::string condition(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>, C> view)
{
    return ygg::visit([&](auto concrete_variant)
                      { return ygg::visit([&](auto concrete_condition) { return condition(names, concrete_condition); }, concrete_variant.get_variant()); },
                      view.get_variant());
}

template<typename C>
std::string effect(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>, C> view)
{
    return ygg::visit([&](auto concrete_variant)
                      { return ygg::visit([&](auto concrete_effect) { return effect(names, concrete_effect); }, concrete_variant.get_variant()); },
                      view.get_variant());
}

inline std::string list_section(std::string_view name, const std::vector<std::string>& values)
{
    return fmt::format("({})", runir::formatting::joined_components(fmt::format(":{}", name), values));
}

template<typename C>
void append_rule(std::ostream& os, FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::base::Rule>, C> view)
{
    auto conditions = std::vector<std::string> {};
    for (auto item : view.get_conditions())
        conditions.push_back(condition(names, item));

    auto effects = std::vector<std::string> {};
    for (auto item : view.get_effects())
        effects.push_back(effect(names, item));

    os << ygg::print_indent << "(:rule\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << runir::kr::ps::base::dl::format::symbol_section(std::string(view.get_symbol().str())) << "\n";
        os << ygg::print_indent << fmt::format("(:description {})", fmt::format("{:?}", std::string(view.get_description().str()))) << "\n";
        os << ygg::print_indent << fmt::format("(:expression {} {})", list_section("conditions", conditions), list_section("effects", effects)) << "\n";
    }
    os << ygg::print_indent << ")";
}

template<typename C>
std::string rule(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::base::Rule>, C> view)
{
    auto os = std::ostringstream {};
    append_rule(os, names, view);
    return os.str();
}

template<typename C>
std::string sketch(ygg::View<ygg::Index<runir::kr::ps::base::Sketch>, C> view)
{
    auto names = declared_features(view);
    auto os = std::ostringstream {};

    os << "(:sketch\n";
    {
        ygg::IndentScope scope(os);

        os << ygg::print_indent << "(:features\n";
        {
            ygg::IndentScope feature_scope(os);
            append_features(os, view.get_context(), names);
        }
        os << ygg::print_indent << ")\n";

        os << ygg::print_indent << "(:rules\n";
        {
            ygg::IndentScope rule_scope(os);
            for (auto item : view.get_rules())
            {
                append_rule(os, names, item);
                os << "\n";
            }
        }
        os << ygg::print_indent << ")\n";
    }
    os << ")\n";

    return os.str();
}

inline std::string sketch_proof_status(runir::kr::ps::base::SketchProofStatus status)
{
    switch (status)
    {
        case runir::kr::ps::base::SketchProofStatus::SUCCESS:
            return "success";
        case runir::kr::ps::base::SketchProofStatus::FAILURE:
            return "failure";
        case runir::kr::ps::base::SketchProofStatus::OUT_OF_TIME:
            return "out_of_time";
        case runir::kr::ps::base::SketchProofStatus::OUT_OF_STATES:
            return "out_of_states";
    }
    return "unknown";
}

template<tyr::planning::TaskKind Kind>
std::string sketch_proof_results(const runir::kr::ps::base::SketchProofResults<Kind>& result)
{
    return fmt::format("SketchProofResults(status={}, graph_vertices={}, graph_edges={}, deadend_transitions={}, open_states={}, cycle={})",
                       sketch_proof_status(result.status),
                       result.graph ? result.graph->get_num_vertices() : 0,
                       result.graph ? result.graph->get_num_edges() : 0,
                       result.deadend_transitions.size(),
                       result.open_states.size(),
                       result.cycle.size());
}

}  // namespace runir::kr::ps::base::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::base::format::feature(view), ctx); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::base::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::base::format::condition(names, view), ctx);
    }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, LanguageTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::base::format::FeatureNames {};
        const auto text =
            ygg::visit([&](auto concrete_condition) { return runir::kr::ps::base::format::condition(names, concrete_condition); }, view.get_variant());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::base::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::base::format::effect(names, view), ctx);
    }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::base::format::FeatureNames {};
        const auto text = ygg::visit([&](auto concrete_effect) { return runir::kr::ps::base::format::effect(names, concrete_effect); }, view.get_variant());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::base::Rule>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::base::Rule>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::base::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::base::format::rule(names, view), ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::base::Sketch>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::base::Sketch>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::base::format::sketch(view), ctx); }
};

template<>
struct fmt::formatter<runir::kr::ps::base::SketchProofStatus> : fmt::formatter<std::string_view>
{
    auto format(runir::kr::ps::base::SketchProofStatus status, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::base::format::sketch_proof_status(status), ctx);
    }
};

template<tyr::planning::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::base::SketchProofResults<Kind>> : fmt::formatter<std::string_view>
{
    auto format(const runir::kr::ps::base::SketchProofResults<Kind>& result, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::base::format::sketch_proof_results(result), ctx);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::base::SketchProofEdgeLabel> : fmt::formatter<std::string_view>
{
    auto format(const runir::kr::ps::base::SketchProofEdgeLabel& label, format_context& ctx) const
    {
        const auto text = fmt::format("rule={} transition={}", std::string(label.rule.get_symbol().str()), label.transition);
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};
#endif

#endif
