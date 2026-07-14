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

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::base::format
{

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>, C> view)
{
    return ygg::visit([](auto concrete_feature) { return runir::kr::ps::base::dl::format::feature(concrete_feature); }, view.get_variant());
}

template<typename Features>
void append_features(std::ostream& os, Features features)
{
    for (auto feature : features)
    {
        os << ygg::print_indent;
        ygg::visit([&](auto concrete_feature) { runir::kr::ps::base::dl::format::append_feature(os, concrete_feature); }, feature.get_variant());
        os << "\n";
    }
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string condition(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return runir::kr::ps::base::dl::format::condition(view);
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string effect(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return runir::kr::ps::base::dl::format::effect(view);
}

template<typename C>
std::string condition(ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>, C> view)
{
    return ygg::visit([](auto concrete_variant)
                      { return ygg::visit([](auto concrete_condition) { return condition(concrete_condition); }, concrete_variant.get_variant()); },
                      view.get_variant());
}

template<typename C>
std::string effect(ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>, C> view)
{
    return ygg::visit([](auto concrete_variant)
                      { return ygg::visit([](auto concrete_effect) { return effect(concrete_effect); }, concrete_variant.get_variant()); },
                      view.get_variant());
}

template<typename Values, typename FormatValue>
void append_list_section(std::ostream& os, std::string_view name, Values values, FormatValue&& format_value)
{
    if (values.empty())
    {
        os << ygg::print_indent << fmt::format("(:{})", name) << "\n";
        return;
    }

    if (values.size() == 1)
    {
        os << ygg::print_indent << "(:" << name << " " << format_value(values.front()) << ")\n";
        return;
    }

    os << ygg::print_indent << fmt::format("(:{}\n", name);
    {
        ygg::IndentScope scope(os);
        for (auto value : values)
            os << ygg::print_indent << format_value(value) << "\n";
    }
    os << ygg::print_indent << ")\n";
}

template<typename C>
void append_rule(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::base::Rule>, C> view)
{
    os << ygg::print_indent << "(:rule\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << runir::kr::ps::base::dl::format::symbol_section(std::string(view.get_symbol().str())) << "\n";
        os << ygg::print_indent << "(:expression\n";
        {
            ygg::IndentScope expression_scope(os);
            append_list_section(os, "conditions", view.get_conditions(), [](auto item) { return condition(item); });
            append_list_section(os, "effects", view.get_effects(), [](auto item) { return effect(item); });
        }
        os << ygg::print_indent << ")\n";
    }
    os << ygg::print_indent << ")";
}

template<typename C>
std::string rule(ygg::View<ygg::Index<runir::kr::ps::base::Rule>, C> view)
{
    auto os = std::ostringstream {};
    append_rule(os, view);
    return os.str();
}

template<typename C>
std::string sketch(ygg::View<ygg::Index<runir::kr::ps::base::Sketch>, C> view)
{
    auto os = std::ostringstream {};

    os << "(:sketch\n";
    {
        ygg::IndentScope scope(os);

        os << ygg::print_indent << "(:features\n";
        {
            ygg::IndentScope feature_scope(os);
            append_features(os, view.template get_features<runir::kr::ps::dl::BooleanFeature>());
            append_features(os, view.template get_features<runir::kr::ps::dl::NumericalFeature>());
        }
        os << ygg::print_indent << ")\n";

        os << ygg::print_indent << "(:rules\n";
        {
            ygg::IndentScope rule_scope(os);
            for (auto item : view.get_rules())
            {
                append_rule(os, item);
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
    return fmt::format("SketchProofResults(status={}, graph_vertices={}, graph_edges={}, deadend_states={}, open_states={}, cycle={})",
                       sketch_proof_status(result.status),
                       result.graph ? result.graph->get_num_vertices() : 0,
                       result.graph ? result.graph->get_num_edges() : 0,
                       result.deadend_states.size(),
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
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::base::format::condition(view), ctx); }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, LanguageTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = ygg::visit([](auto concrete_condition) { return runir::kr::ps::base::format::condition(concrete_condition); }, view.get_variant());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::base::format::effect(view), ctx); }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = ygg::visit([](auto concrete_effect) { return runir::kr::ps::base::format::effect(concrete_effect); }, view.get_variant());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::base::Rule>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::base::Rule>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::base::format::rule(view), ctx); }
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
