#ifndef RUNIR_SRC_KR_PARSER_RESOLUTION_HPP_
#define RUNIR_SRC_KR_PARSER_RESOLUTION_HPP_

#include "runir/kr/errors.hpp"
#include "runir/kr/parser/ast.hpp"
#include "runir/kr/parser/diagnostics.hpp"

#include <cstddef>
#include <optional>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace runir::kr::parser
{

template<tyr::formalism::FactKind T>
auto require_predicate(tyr::formalism::planning::DomainView domain,
                       const ast::Identifier& name,
                       size_t arity,
                       const char* constructor_name,
                       const DiagnosticContext& diagnostics)
{
    for (auto predicate : domain.template get_predicates<T>())
    {
        if (predicate.get_name() != name.text)
            continue;
        if (predicate.get_arity() != arity)
            diagnostics.throw_at(name, ArityMismatchError(constructor_name, arity, predicate.get_arity()));
        return std::optional(predicate);
    }
    return std::optional<tyr::formalism::planning::PredicateView<T>> {};
}

template<typename Make>
auto resolve_predicate(tyr::formalism::planning::DomainView domain,
                       const ast::Identifier& name,
                       size_t arity,
                       const char* constructor_name,
                       const DiagnosticContext& diagnostics,
                       Make make)
{
    if (auto predicate = require_predicate<tyr::formalism::StaticTag>(domain, name, arity, constructor_name, diagnostics))
        return make(tyr::formalism::StaticTag {}, *predicate);
    if (auto predicate = require_predicate<tyr::formalism::FluentTag>(domain, name, arity, constructor_name, diagnostics))
        return make(tyr::formalism::FluentTag {}, *predicate);
    if (auto predicate = require_predicate<tyr::formalism::DerivedTag>(domain, name, arity, constructor_name, diagnostics))
        return make(tyr::formalism::DerivedTag {}, *predicate);

    diagnostics.throw_at(name, UndefinedSymbolError("predicate", name.text));
}

inline tyr::formalism::planning::ObjectView
require_object(tyr::formalism::planning::DomainView domain, const ast::Identifier& name, const DiagnosticContext& diagnostics)
{
    for (auto object : domain.get_constants())
        if (object.get_name() == name.text)
            return object;

    diagnostics.throw_at(name, UndefinedSymbolError("constant", name.text));
}

/// The constants of these names, appended to a builder's list of object children.
template<typename Identifier>
void append_objects(tyr::formalism::planning::DomainView domain,
                    const urd::Store<urd::sequence<Identifier>>& names,
                    const DiagnosticContext& diagnostics,
                    std::vector<urd::child<tyr::formalism::Object, tyr::formalism::planning::Repository>>& result)
{
    result.reserve(result.size() + names.size());
    for (const auto& name : names)
        urd::append(result, require_object(domain, name, diagnostics));
}

}  // namespace runir::kr::parser

#endif
