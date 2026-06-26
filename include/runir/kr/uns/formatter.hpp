#ifndef RUNIR_KR_UNS_FORMATTER_HPP_
#define RUNIR_KR_UNS_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/kr/uns/classifier_view.hpp"
#include "runir/kr/uns/dl/formatter.hpp"
#include "runir/kr/uns/feature_view.hpp"

#include <fmt/format.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::uns::format
{

inline void append_value(std::ostream& os, std::string_view value)
{
    auto stream = std::istringstream(std::string(value));
    auto line = std::string {};
    auto first = true;
    while (std::getline(stream, line))
    {
        if (!first)
            os << '\n';
        os << ygg::print_indent << line;
        first = false;
    }
}

inline void append_value_section(std::ostream& os, std::string_view name, std::string_view value)
{
    const auto formatted_value = runir::pretty_sexpression(value);
    if (formatted_value.find('\n') == std::string::npos)
    {
        os << ygg::print_indent << fmt::format("(:{} {})", name, formatted_value) << "\n";
        return;
    }

    os << ygg::print_indent << fmt::format("(:{}\n", name);
    {
        ygg::IndentScope scope(os);
        append_value(os, formatted_value);
        os << "\n";
    }
    os << ygg::print_indent << ")\n";
}

template<typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::uns::Feature>, C> view)
{
    return ygg::visit([](auto concrete) { return runir::kr::uns::dl::format::feature(concrete); }, view.get_variant());
}

template<typename C>
std::string feature_symbol(ygg::View<ygg::Index<runir::kr::uns::Feature>, C> view)
{
    return ygg::visit([](auto concrete) { return std::string(concrete.get_symbol().str()); }, view.get_variant());
}

template<typename C>
std::string literal(ygg::View<ygg::Index<runir::kr::uns::ClassifierLiteral>, C> view)
{
    const auto symbol = ygg::visit([](auto wrapper) { return feature_symbol(wrapper); }, view.get_feature());
    return view.get_polarity() ? symbol : fmt::format("(not {})", symbol);
}

template<typename C>
std::string clause(ygg::View<ygg::Index<runir::kr::uns::ClassifierClause>, C> view)
{
    auto os = std::ostringstream {};
    os << "(and";
    for (auto item : view.get_literals())
        os << ' ' << literal(item);
    os << ")";
    return os.str();
}

template<typename C>
std::string classifier(ygg::View<ygg::Index<runir::kr::uns::Classifier>, C> view)
{
    auto os = std::ostringstream {};
    os << "(:classifier\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << fmt::format("(:symbol {})", std::string(view.get_symbol().str())) << "\n";

        os << ygg::print_indent << "(:features\n";
        {
            ygg::IndentScope feature_scope(os);
            for (auto item : view.get_features())
            {
                runir::kr::uns::dl::format::append_component(os, feature(item));
                os << "\n";
            }
        }
        os << ygg::print_indent << ")\n";

        auto expression = std::string("(or");
        for (auto item : view.get_clauses())
            expression += fmt::format(" {}", clause(item));
        expression += ")";
        append_value_section(os, "expression", expression);
    }
    os << ")";
    return os.str();
}

}  // namespace runir::kr::uns::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::uns::Feature>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::uns::Feature>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::uns::format::feature(view), ctx); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::uns::ClassifierLiteral>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::uns::ClassifierLiteral>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::uns::format::literal(view), ctx); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::uns::ClassifierClause>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::uns::ClassifierClause>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::uns::format::clause(view), ctx); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::uns::Classifier>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::uns::Classifier>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::uns::format::classifier(view), ctx); }
};
#endif

#endif
