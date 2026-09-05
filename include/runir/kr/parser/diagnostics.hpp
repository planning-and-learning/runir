#ifndef RUNIR_KR_PARSER_DIAGNOSTICS_HPP_
#define RUNIR_KR_PARSER_DIAGNOSTICS_HPP_

#include "runir/kr/errors.hpp"
#include "runir/kr/parser/error_handler.hpp"

#include <functional>
#include <optional>
#include <string>

namespace runir::kr::parser
{

struct DiagnosticContext
{
    std::optional<std::reference_wrapper<const ErrorHandlerType>> active;

    struct Scope
    {
        Scope(DiagnosticContext& diagnostics, const ErrorHandlerType& error_handler) : diagnostics(diagnostics), previous(diagnostics.active)
        {
            diagnostics.active = std::cref(error_handler);
        }
        ~Scope() { diagnostics.active = previous; }

        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;

        DiagnosticContext& diagnostics;
        std::optional<std::reference_wrapper<const ErrorHandlerType>> previous;
    };

    static ParseError parse_error(const ErrorHandlerType& error_handler, const std::string& fallback, Iterator position)
    {
        auto error = ParseError(fallback);
        error.set_diagnostic(error_handler.diagnostic(fallback, position));
        return error;
    }

    template<typename Node, typename Error>
    [[noreturn]] void throw_at(const Node& node, Error error) const
    {
        if (active)
            error.set_diagnostic(active->get().make_diagnostic(node, error.message()));
        throw error;
    }

    template<typename Error>
    [[noreturn]] void throw_at(Iterator position, Error error) const
    {
        if (active)
            error.set_diagnostic(active->get().make_diagnostic(position, error.message()));
        throw error;
    }
};

}  // namespace runir::kr::parser

#endif
