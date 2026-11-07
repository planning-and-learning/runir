#ifndef RUNIR_KR_ERRORS_HPP_
#define RUNIR_KR_ERRORS_HPP_

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <yggdrasil/diagnostics/errors.hpp>

#if defined(__clang__) || defined(__GNUC__)
#define RUNIR_KR_ERROR_API __attribute__((visibility("default")))
#else
#define RUNIR_KR_ERROR_API
#endif

namespace runir::kr
{

class RUNIR_KR_ERROR_API UndefinedSymbolError : public ygg::diagnostics::SemanticError
{
public:
    UndefinedSymbolError(const std::string& kind, const std::string& name) : ygg::diagnostics::SemanticError("Undefined " + kind + ": " + name) {}
};

class RUNIR_KR_ERROR_API DuplicateDefinitionError : public ygg::diagnostics::SemanticError
{
public:
    DuplicateDefinitionError(const std::string& kind, const std::string& name) : ygg::diagnostics::SemanticError("Duplicate " + kind + " definition: " + name) {}
};

class RUNIR_KR_ERROR_API ArityMismatchError : public ygg::diagnostics::SemanticError
{
public:
    ArityMismatchError(const std::string& subject, std::size_t expected, std::size_t actual) :
        ygg::diagnostics::SemanticError("Arity mismatch for " + subject + ": expected " + std::to_string(expected) + ", got " + std::to_string(actual))
    {
    }
};

class RUNIR_KR_ERROR_API InvalidExpressionError : public ygg::diagnostics::SemanticError
{
public:
    explicit InvalidExpressionError(std::string message) : ygg::diagnostics::SemanticError(std::move(message)) {}
};

}  // namespace runir::kr

#undef RUNIR_KR_ERROR_API

#endif
