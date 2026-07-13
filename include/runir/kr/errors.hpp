#ifndef RUNIR_KR_ERRORS_HPP_
#define RUNIR_KR_ERRORS_HPP_

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

#if defined(__clang__) || defined(__GNUC__)
#define RUNIR_KR_ERROR_API __attribute__((visibility("default")))
#else
#define RUNIR_KR_ERROR_API
#endif

namespace runir::kr
{

class RUNIR_KR_ERROR_API SemanticError : public std::runtime_error
{
public:
    explicit SemanticError(std::string message) : std::runtime_error(message), m_message(std::move(message)), m_display_message(m_message) {}

    const char* what() const noexcept override { return m_display_message.c_str(); }
    const std::string& message() const noexcept { return m_message; }
    void set_display_message(std::string display_message) { m_display_message = std::move(display_message); }

private:
    std::string m_message;
    std::string m_display_message;
};

class RUNIR_KR_ERROR_API ParseError : public SemanticError
{
public:
    explicit ParseError(std::string message) : SemanticError(std::move(message)) {}
};

class RUNIR_KR_ERROR_API UndefinedSymbolError : public SemanticError
{
public:
    UndefinedSymbolError(const std::string& kind, const std::string& name) : SemanticError("Undefined " + kind + ": " + name) {}
};

class RUNIR_KR_ERROR_API DuplicateDefinitionError : public SemanticError
{
public:
    DuplicateDefinitionError(const std::string& kind, const std::string& name) : SemanticError("Duplicate " + kind + " definition: " + name) {}
};

class RUNIR_KR_ERROR_API ArityMismatchError : public SemanticError
{
public:
    ArityMismatchError(const std::string& subject, std::size_t expected, std::size_t actual) :
        SemanticError("Arity mismatch for " + subject + ": expected " + std::to_string(expected) + ", got " + std::to_string(actual))
    {
    }
};

class RUNIR_KR_ERROR_API InvalidExpressionError : public SemanticError
{
public:
    explicit InvalidExpressionError(std::string message) : SemanticError(std::move(message)) {}
};

}  // namespace runir::kr

#undef RUNIR_KR_ERROR_API

#endif
