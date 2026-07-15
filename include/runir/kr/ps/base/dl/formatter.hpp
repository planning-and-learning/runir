#ifndef RUNIR_KR_PS_BASE_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_BASE_DL_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/kr/ps/base/dl/feature_view.hpp"
#include "runir/kr/ps/base/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/base/dl/structural_termination_data.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/dl/formatter.hpp"

#include <fmt/format.h>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <yggdrasil/formatting/dynamic_bitset_formatters.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::base::dl::format
{

inline std::string symbol_section(std::string_view value) { return fmt::format("(:symbol {})", value); }

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view)
{
    os << "(:" << FeatureTag::keyword << "\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << symbol_section(std::string(view.get_symbol().str())) << "\n";
        os << ygg::print_indent << "(:expression ";
        fmt::format_to(std::ostream_iterator<char>(os), "{}", view.get_expression());
        os << ")\n";
    }
    os << ygg::print_indent << ")";
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view)
{
    auto os = std::ostringstream {};
    append_feature(os, view);
    return os.str();
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string condition(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return fmt::format("({} {})", ObservationTag::keyword, view.get_feature().get_symbol().str());
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string effect(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return fmt::format("({} {})", ObservationTag::keyword, view.get_feature().get_symbol().str());
}

}  // namespace runir::kr::ps::base::dl::format

template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::base::dl::format::feature(view), ctx); }
};

template<typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::base::dl::format::condition(view), ctx);
    }
};

template<typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::base::dl::format::effect(view), ctx); }
};

template<>
struct fmt::formatter<runir::kr::ps::base::dl::PolicyGraphVertexLabel, char> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::PolicyGraphVertexLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("(booleans={}, numericals={})", label.boolean_values, label.numerical_values);
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::base::dl::PolicyGraphEdgeLabel, char> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::PolicyGraphEdgeLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("(rule={}, numerical_changes={})", label.rule.get_symbol(), label.numerical_changes);
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::base::dl::StructuralTerminationResult, char> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::StructuralTerminationResult& result, FormatContext& ctx) const
    {
        const auto text = result.is_terminating() ? std::string { "StructuralTerminationResult(terminating)" } :
                                                    fmt::format("StructuralTerminationResult(non-terminating, counterexample with {} vertices and {} edges)",
                                                                result.counterexample->get_num_vertices(),
                                                                result.counterexample->get_num_edges());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::base::dl::IncompleteStructuralTerminationResult, char> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::IncompleteStructuralTerminationResult& result, FormatContext& ctx) const
    {
        const auto text = result.is_terminating() ?
                              std::string { "IncompleteStructuralTerminationResult(terminating)" } :
                              fmt::format("IncompleteStructuralTerminationResult(unknown, {} surviving rules)", result.surviving_rules.size());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

#endif
