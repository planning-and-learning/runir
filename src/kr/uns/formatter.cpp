#include "runir/kr/uns/formatter.hpp"

#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/formatter.hpp"
#include "runir/kr/uns/classifier_view.hpp"
#include "runir/kr/uns/repository.hpp"

#include <fmt/format.h>
#include <urd/format.hpp>
#include <variant>
#include <yggdrasil/formatting/cista_formatters.hpp>

namespace ru = runir::kr::uns;

fmt::format_context::iterator fmt::formatter<ru::ClassifierLiteralView>::format(const ru::ClassifierLiteralView& value, fmt::format_context& context) const
{
    const auto symbol =
        std::visit([](auto feature) { return std::visit([](auto concrete) { return concrete.get_symbol(); }, feature.get_variant()); }, value.get_feature());
    if (value.get_polarity())
        return fmt::format_to(context.out(), "{}", symbol.str());
    return fmt::format_to(context.out(), "(not {})", symbol.str());
}

fmt::format_context::iterator fmt::formatter<ru::ClassifierClauseView>::format(const ru::ClassifierClauseView& value, fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(and");
    for (auto literal : value.get_literals())
        out = fmt::format_to(out, " {}", literal);
    return fmt::format_to(out, ")");
}

fmt::format_context::iterator fmt::formatter<ru::ClassifierView>::format(const ru::ClassifierView& value, fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:classifier");
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:symbol {})", urd::new_line(), value.get_symbol());
        out = fmt::format_to(out, "{}(:features", urd::new_line());
        {
            const auto features = urd::scoped_indent();
            for (auto feature : value.get_features())
                out = fmt::format_to(out, "{}{}", urd::new_line(), feature);
        }
        out = fmt::format_to(out, "{})", urd::new_line());
        if (value.get_clauses().empty())
            out = fmt::format_to(out, "{}(:expression (or))", urd::new_line());
        else
        {
            out = fmt::format_to(out, "{}(:expression", urd::new_line());
            {
                const auto expression = urd::scoped_indent();
                out = fmt::format_to(out, "{}(or", urd::new_line());
                {
                    const auto clauses = urd::scoped_indent();
                    for (auto clause : value.get_clauses())
                        out = fmt::format_to(out, "{}{}", urd::new_line(), clause);
                }
                out = fmt::format_to(out, "{})", urd::new_line());
            }
            out = fmt::format_to(out, "{})", urd::new_line());
        }
    }
    return fmt::format_to(out, "{})", urd::new_line());
}
