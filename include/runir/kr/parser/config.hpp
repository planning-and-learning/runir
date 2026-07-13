#ifndef RUNIR_KR_PARSER_CONFIG_HPP_
#define RUNIR_KR_PARSER_CONFIG_HPP_

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <functional>
#include <optional>
#include <ostream>
#include <string>
#include <utility>

namespace runir::kr::parser
{
namespace x3 = boost::spirit::x3;

using Iterator = std::string::const_iterator;

template<typename It>
class ErrorHandler : public x3::error_handler<It>
{
public:
    using Base = x3::error_handler<It>;

    struct Diagnostic
    {
        It position;
        std::string message;
    };

    ErrorHandler(It first, It last, std::ostream& output, int tabs = 4) : Base(first, last, output, "", tabs), m_tabs(tabs) {}

    int tabs() const noexcept { return m_tabs; }
    void clear_error() noexcept { m_error.reset(); }
    void record_error(It position, std::string message)
    {
        if (!m_error)
            m_error = Diagnostic { position, std::move(message) };
    }
    const std::optional<Diagnostic>& error() const noexcept { return m_error; }

private:
    int m_tabs;
    std::optional<Diagnostic> m_error;
};

using ErrorHandlerTag = x3::error_handler_tag;
using ErrorHandlerType = ErrorHandler<Iterator>;

inline auto skipper() { return x3::ascii::space | (';' >> *(x3::char_ - x3::eol) >> (x3::eol | x3::eoi)); }

using PhraseContext = x3::phrase_parse_context<decltype(skipper())>::type;
using Context = x3::context<ErrorHandlerTag, std::reference_wrapper<ErrorHandlerType>, PhraseContext>;

}  // namespace runir::kr::parser

#endif
