#include "runir/kr/ps/ext/dl/parser/parser.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <cctype>
#include <cstddef>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext::dl::parser
{
namespace
{

namespace x3 = boost::spirit::x3;

using iterator_type = std::string::const_iterator;
using error_handler_type = x3::error_handler<iterator_type>;

struct X3ErrorHandlerBase
{
    template<typename Iterator, typename Exception, typename Context>
    x3::error_handler_result on_error(Iterator&, const Iterator&, const Exception& x, const Context& context)
    {
        auto& error_handler = x3::get<x3::error_handler_tag>(context).get();
        error_handler(x.where(), "Error! Expecting: " + std::string(x.which()) + " here:");
        return x3::error_handler_result::fail;
    }
};

struct SExpressionClass
{
};
struct SExpressionRootClass : X3ErrorHandlerBase
{
};

x3::rule<SExpressionClass> const s_expression = "S-expression";
x3::rule<SExpressionRootClass> const s_expression_root = "module S-expression";

const auto quoted_string_def = x3::lexeme[x3::lit('"') > *((x3::lit('\\') > x3::char_) | (x3::char_ - '"')) > x3::lit('"')];
const auto atom_def = x3::lexeme[+(x3::char_ - x3::ascii::space - '(' - ')' - '"')];
const auto s_expression_def = quoted_string_def | atom_def | (x3::lit('(') > *s_expression > x3::lit(')'));
const auto s_expression_root_def = s_expression > x3::eoi;

BOOST_SPIRIT_DEFINE(s_expression, s_expression_root)

void validate_s_expression_syntax_with_x3(const std::string& description)
{
    auto first = description.cbegin();
    const auto last = description.cend();

    auto errors = std::ostringstream {};
    auto error_handler = error_handler_type(first, last, errors);
    const auto parser = x3::with<x3::error_handler_tag>(std::ref(error_handler))[s_expression_root];
    const auto success = x3::phrase_parse(first, last, parser, x3::ascii::space);

    if (!success || first != last)
    {
        auto message = errors.str();
        if (message.empty())
            message = "Failed to parse module S-expression.";
        throw std::runtime_error(message);
    }
}

struct Node
{
    bool list = false;
    bool quoted = false;
    std::size_t source_offset = 0;
    std::size_t atom_offset = 0;
    std::string atom;
    std::vector<Node> children;
};

class Reader
{
private:
    std::string_view m_input;
    std::size_t m_pos = 0;

public:
    explicit Reader(std::string_view input) : m_input(input) {}

    Node read_root()
    {
        skip_ws();
        auto result = read_node();
        skip_ws();
        if (m_pos != m_input.size())
            fail("Unexpected trailing input");
        return result;
    }

private:
    [[noreturn]] void fail(const char* message) const { throw std::runtime_error(std::string(message) + " at offset " + std::to_string(m_pos) + "."); }

    void skip_ws()
    {
        while (m_pos < m_input.size() && std::isspace(static_cast<unsigned char>(m_input[m_pos])))
            ++m_pos;
    }

    Node read_node()
    {
        skip_ws();
        if (m_pos >= m_input.size())
            fail("Unexpected end of input");
        if (m_input[m_pos] == '(')
            return read_list();
        if (m_input[m_pos] == '"')
            return read_quoted();
        return read_atom();
    }

    Node read_list()
    {
        const auto offset = m_pos;
        ++m_pos;
        auto result = Node {};
        result.list = true;
        result.source_offset = offset;
        result.atom_offset = offset;
        while (true)
        {
            skip_ws();
            if (m_pos >= m_input.size())
                fail("Unclosed list");
            if (m_input[m_pos] == ')')
            {
                ++m_pos;
                return result;
            }
            result.children.push_back(read_node());
        }
    }

    Node read_quoted()
    {
        const auto offset = m_pos;
        ++m_pos;
        auto result = Node {};
        result.quoted = true;
        result.source_offset = offset;
        result.atom_offset = offset;
        while (m_pos < m_input.size())
        {
            const auto ch = m_input[m_pos++];
            if (ch == '"')
                return result;
            if (ch == '\\')
            {
                if (m_pos >= m_input.size())
                    fail("Unclosed escape sequence");
                result.atom.push_back(m_input[m_pos++]);
            }
            else
            {
                result.atom.push_back(ch);
            }
        }
        fail("Unclosed quoted string");
    }

    Node read_atom()
    {
        const auto offset = m_pos;
        auto result = Node {};
        result.source_offset = offset;
        result.atom_offset = offset;
        while (m_pos < m_input.size())
        {
            const auto ch = m_input[m_pos];
            if (std::isspace(static_cast<unsigned char>(ch)) || ch == '(' || ch == ')')
                break;
            result.atom.push_back(ch);
            ++m_pos;
        }
        if (result.atom.empty())
            fail("Expected atom");
        return result;
    }
};

[[noreturn]] void fail_at(const std::string& message, std::size_t offset) { throw std::runtime_error(message + " at offset " + std::to_string(offset) + "."); }

const Node& require_child(const Node& node, std::size_t index, std::string_view context)
{
    if (!node.list || index >= node.children.size())
        fail_at(std::string("Malformed ") + std::string(context) + ".", node.source_offset);
    return node.children[index];
}

std::string atom(const Node& node, std::string_view context)
{
    if (node.list)
        fail_at(std::string("Expected atom in ") + std::string(context) + ".", node.source_offset);
    return node.atom;
}

std::string head(const Node& node, std::string_view context) { return atom(require_child(node, 0, context), context); }

std::uint64_t integer(const Node& node, std::string_view context)
{
    const auto text = atom(node, context);
    std::size_t read = 0;
    try
    {
        const auto result = std::stoull(text, &read);
        if (read != text.size())
            fail_at(std::string("Expected integer in ") + std::string(context) + ".", node.atom_offset);
        return result;
    }
    catch (const std::exception&)
    {
        fail_at(std::string("Expected integer in ") + std::string(context) + ".", node.atom_offset);
    }
}

std::string render(const Node& node)
{
    if (!node.list)
    {
        if (!node.quoted)
            return node.atom;
        auto result = std::string("\"");
        for (const auto ch : node.atom)
        {
            if (ch == '"' || ch == '\\')
                result.push_back('\\');
            result.push_back(ch);
        }
        result.push_back('"');
        return result;
    }

    auto result = std::string("(");
    for (std::size_t i = 0; i < node.children.size(); ++i)
    {
        if (i != 0)
            result.push_back(' ');
        result += render(node.children[i]);
    }
    result.push_back(')');
    return result;
}

ast::ArgumentKind parse_argument_kind(const Node& node)
{
    const auto value = atom(node, "argument kind");
    if (value == ":concept")
        return ast::ArgumentKind::CONCEPT;
    if (value == ":role")
        return ast::ArgumentKind::ROLE;
    if (value == ":boolean")
        return ast::ArgumentKind::BOOLEAN;
    if (value == ":numerical")
        return ast::ArgumentKind::NUMERICAL;
    fail_at("Unknown argument kind: " + value + ".", node.atom_offset);
}

ast::FeatureKind parse_feature_kind(const Node& node)
{
    const auto value = atom(node, "feature kind");
    if (value == ":concept")
        return ast::FeatureKind::CONCEPT;
    if (value == ":boolean")
        return ast::FeatureKind::BOOLEAN;
    if (value == ":numerical")
        return ast::FeatureKind::NUMERICAL;
    fail_at("Unknown feature kind: " + value + ".", node.atom_offset);
}

ast::ObservationKind parse_observation_kind(const Node& node)
{
    const auto value = atom(node, "observation kind");
    if (value == ":positive")
        return ast::ObservationKind::POSITIVE;
    if (value == ":negative")
        return ast::ObservationKind::NEGATIVE;
    if (value == ":equal_zero")
        return ast::ObservationKind::EQUAL_ZERO;
    if (value == ":greater_zero")
        return ast::ObservationKind::GREATER_ZERO;
    if (value == ":unchanged")
        return ast::ObservationKind::UNCHANGED;
    if (value == ":increases")
        return ast::ObservationKind::INCREASES;
    if (value == ":decreases")
        return ast::ObservationKind::DECREASES;
    fail_at("Unknown observation kind: " + value + ".", node.atom_offset);
}

ast::RuleKind parse_rule_kind(const Node& node)
{
    const auto value = atom(node, "rule kind");
    if (value == ":load")
        return ast::RuleKind::LOAD;
    if (value == ":sketch")
        return ast::RuleKind::SKETCH;
    if (value == ":do")
        return ast::RuleKind::DO;
    if (value == ":call")
        return ast::RuleKind::CALL;
    fail_at("Unknown rule kind: " + value + ".", node.atom_offset);
}

ast::Observation parse_observation(const Node& node)
{
    if (!node.list || node.children.size() != 2)
        fail_at("Malformed observation.", node.source_offset);
    return ast::Observation { node.source_offset, node.children[1].source_offset, parse_observation_kind(node.children[0]), render(node.children[1]) };
}

std::vector<ast::Observation> parse_observation_section(const Node& section, std::string_view name)
{
    if (head(section, name) != name)
        fail_at(std::string("Expected section ") + std::string(name) + ".", section.source_offset);
    auto result = std::vector<ast::Observation> {};
    for (std::size_t i = 1; i < section.children.size(); ++i)
        result.push_back(parse_observation(section.children[i]));
    return result;
}

bool is_allowed_rule_expression_section(ast::RuleKind kind, std::string_view name);

bool is_rule_metadata_section(std::string_view name) { return name == ":symbol" || name == ":description"; }

bool is_allowed_rule_section(ast::RuleKind kind, std::string_view name)
{
    return is_rule_metadata_section(name) || name == ":expression" || is_allowed_rule_expression_section(kind, name);
}

bool is_allowed_rule_expression_section(ast::RuleKind kind, std::string_view name)
{
    if (name == ":conditions")
        return true;

    switch (kind)
    {
        case ast::RuleKind::LOAD:
            return name == ":concept" || name == ":register";
        case ast::RuleKind::SKETCH:
            return name == ":effects";
        case ast::RuleKind::DO:
            return name == ":action" || name == ":arguments" || name == ":effects";
        case ast::RuleKind::CALL:
            return name == ":callee" || name == ":arguments";
    }
    return false;
}

void validate_rule_sections(const Node& rule, ast::RuleKind kind)
{
    auto seen = std::unordered_set<std::string> {};
    for (std::size_t i = 1; i < rule.children.size(); ++i)
    {
        const auto& section = rule.children[i];
        if (!section.list || section.children.empty())
            fail_at("Malformed rule section.", section.source_offset);

        const auto name = atom(section.children[0], "rule section");
        if (!is_allowed_rule_section(kind, name))
            fail_at("Rule section " + name + " is not valid.", section.source_offset);
        if (!seen.emplace(name).second)
            fail_at("Duplicate rule section " + name + ".", section.source_offset);
    }
}

bool is_allowed_feature_section(std::string_view name) { return name == ":symbol" || name == ":description" || name == ":expression"; }

void validate_feature_sections(const Node& feature)
{
    auto seen = std::unordered_set<std::string> {};
    for (std::size_t i = 1; i < feature.children.size(); ++i)
    {
        const auto& section = feature.children[i];
        if (!section.list || section.children.empty())
            fail_at("Malformed feature section.", section.source_offset);

        const auto name = atom(section.children[0], "feature section");
        if (!is_allowed_feature_section(name))
            fail_at("Feature section " + name + " is not valid.", section.source_offset);
        if (!seen.emplace(name).second)
            fail_at("Duplicate feature section " + name + ".", section.source_offset);
    }
}

const Node* find_section(const Node& rule, std::string_view name)
{
    for (std::size_t i = 1; i < rule.children.size(); ++i)
        if (rule.children[i].list && !rule.children[i].children.empty() && atom(rule.children[i].children[0], "rule section") == name)
            return &rule.children[i];
    return nullptr;
}

const Node& require_section(const Node& rule, std::string_view name)
{
    if (const auto* section = find_section(rule, name))
        return *section;
    fail_at(std::string("Missing rule section ") + std::string(name) + ".", rule.source_offset);
}

void validate_rule_expression_sections(const Node& expression, ast::RuleKind kind, bool allow_metadata_sections = false)
{
    auto seen = std::unordered_set<std::string> {};
    for (std::size_t i = 1; i < expression.children.size(); ++i)
    {
        const auto& section = expression.children[i];
        if (!section.list || section.children.empty())
            fail_at("Malformed rule expression section.", section.source_offset);
        const auto name = atom(section.children[0], "rule expression section");
        if (allow_metadata_sections && is_rule_metadata_section(name))
            continue;
        if (!is_allowed_rule_expression_section(kind, name))
            fail_at("Rule expression section " + name + " is not valid for this rule kind.", section.source_offset);
        if (!seen.emplace(name).second)
            fail_at("Duplicate rule expression section " + name + ".", section.source_offset);
    }
}

const Node& require_rule_expression(const Node& rule, ast::RuleKind kind)
{
    if (const auto* expression = find_section(rule, ":expression"))
    {
        for (std::size_t i = 1; i < rule.children.size(); ++i)
        {
            const auto& section = rule.children[i];
            const auto name = atom(section.children[0], "rule section");
            if (!is_rule_metadata_section(name) && name != ":expression")
                fail_at("Rule must not mix direct expression sections with an :expression wrapper.", section.source_offset);
        }
        validate_rule_expression_sections(*expression, kind);
        return *expression;
    }

    auto has_expression_section = false;
    for (std::size_t i = 1; i < rule.children.size(); ++i)
    {
        const auto& section = rule.children[i];
        const auto name = atom(section.children[0], "rule section");
        if (is_rule_metadata_section(name))
            continue;
        has_expression_section = true;
        if (!is_allowed_rule_expression_section(kind, name))
            fail_at("Rule expression section " + name + " is not valid for this rule kind.", section.source_offset);
    }

    if (!has_expression_section)
        fail_at("Missing rule expression sections.", rule.source_offset);

    validate_rule_expression_sections(rule, kind, true);
    return rule;
}

const Node& require_single_value_section_value(const Node& rule, std::string_view name)
{
    const auto& section = require_section(rule, name);
    if (section.children.size() != 2)
        fail_at(std::string("Malformed rule section ") + std::string(name) + ".", section.source_offset);
    return section.children[1];
}

std::string optional_metadata_value(const Node& node, std::string_view name);
std::string parse_symbol_section_value(const Node& section, std::string_view name);
std::string optional_symbol_value(const Node& node, std::string_view name);
const Node* optional_single_value_section(const Node& node, std::string_view name);

std::vector<ast::Expression> parse_expression_list_section(const Node& rule, std::string_view name)
{
    const auto& section = require_section(rule, name);
    auto result = std::vector<ast::Expression> {};
    for (std::size_t i = 1; i < section.children.size(); ++i)
        result.push_back(ast::Expression { section.children[i].source_offset, render(section.children[i]) });
    return result;
}

ast::Rule parse_rule(const Node& node)
{
    if (!node.list || node.children.empty())
        fail_at("Malformed rule.", node.source_offset);
    auto result = ast::Rule {};
    result.source_offset = node.source_offset;
    result.kind = parse_rule_kind(node.children[0]);
    validate_rule_sections(node, result.kind);
    if (result.kind != ast::RuleKind::CALL)
    {
        const auto& symbol_section = require_section(node, ":symbol");
        result.symbol = parse_symbol_section_value(symbol_section, ":symbol");
        result.description = optional_metadata_value(node, ":description");
    }
    const auto& expression = require_rule_expression(node, result.kind);
    result.conditions = parse_observation_section(require_section(expression, ":conditions"), ":conditions");

    switch (result.kind)
    {
        case ast::RuleKind::LOAD:
        {
            const auto& concept_node = require_single_value_section_value(expression, ":concept");
            result.concept_expression = render(concept_node);
            result.concept_expression_offset = concept_node.source_offset;
        }
            {
                const auto& reg = require_single_value_section_value(expression, ":register");
                result.reg = std::to_string(integer(reg, ":register"));
                result.register_offset = reg.atom_offset;
            }
            break;
        case ast::RuleKind::SKETCH:
            result.effects = parse_observation_section(require_section(expression, ":effects"), ":effects");
            break;
        case ast::RuleKind::DO:
        {
            const auto& action = require_single_value_section_value(expression, ":action");
            result.action = atom(action, ":action");
            result.action_offset = action.atom_offset;
        }
            result.arguments_offset = require_section(expression, ":arguments").source_offset;
            result.arguments = parse_expression_list_section(expression, ":arguments");
            if (const auto* effects = find_section(expression, ":effects"))
                result.effects = parse_observation_section(*effects, ":effects");
            break;
        case ast::RuleKind::CALL:
        {
            const auto& callee = require_single_value_section_value(expression, ":callee");
            result.callee = atom(callee, ":callee");
            result.callee_offset = callee.atom_offset;
            result.arguments_offset = require_section(expression, ":arguments").source_offset;
            result.arguments = parse_expression_list_section(expression, ":arguments");
            break;
        }
    }

    return result;
}

ast::Argument parse_argument(const Node& node)
{
    if (!node.list || node.children.size() != 3)
        fail_at("Malformed argument.", node.source_offset);
    return ast::Argument { node.source_offset,
                           node.children[1].atom_offset,
                           parse_argument_kind(node.children[0]),
                           atom(node.children[1], "argument"),
                           integer(node.children[2], "argument") };
}

ast::Register parse_register(const Node& node)
{
    if (!node.list || node.children.empty() || atom(node.children[0], "register") != ":concept")
        fail_at("Malformed register.", node.source_offset);

    auto seen = std::unordered_set<std::string> {};
    for (std::size_t i = 1; i < node.children.size(); ++i)
    {
        const auto& section = node.children[i];
        if (!section.list || section.children.empty())
            fail_at("Malformed register section.", section.source_offset);
        const auto name = atom(section.children[0], "register section");
        if (name != ":symbol")
            fail_at("Register section " + name + " is not valid.", section.source_offset);
        if (!seen.emplace(name).second)
            fail_at("Duplicate register section " + name + ".", section.source_offset);
    }

    const auto& symbol = require_single_value_section_value(node, ":symbol");
    const auto identifier = integer(symbol, "register");
    return ast::Register { node.source_offset, symbol.atom_offset, std::to_string(identifier), identifier };
}

std::string optional_metadata_value(const Node& node, std::string_view name)
{
    if (const auto* section = find_section(node, name))
    {
        if (section->children.size() != 2)
            fail_at("Malformed metadata section " + std::string(name) + ".", section->source_offset);
        return atom(section->children[1], name);
    }
    return {};
}

bool is_symbol_atom(std::string_view value)
{
    if (value.empty())
        return true;

    const auto first = static_cast<unsigned char>(value.front());
    if (!std::isalpha(first) && value.front() != '_')
        return false;

    for (const auto ch : value.substr(1))
    {
        const auto value_ch = static_cast<unsigned char>(ch);
        if (!std::isalnum(value_ch) && ch != '_' && ch != '-')
            return false;
    }
    return true;
}

std::string parse_symbol_section_value(const Node& section, std::string_view name)
{
    if (!section.list || section.children.empty() || atom(section.children[0], "symbol section") != name)
        fail_at("Malformed symbol section " + std::string(name) + ".", section.source_offset);
    if (section.children.size() != 2)
        fail_at("Malformed symbol section " + std::string(name) + ".", section.source_offset);

    const auto& value = section.children[1];
    if (value.list || value.quoted)
        fail_at("Symbol section " + std::string(name) + " expects an unquoted atom.", value.source_offset);

    const auto result = atom(value, name);
    if (!is_symbol_atom(result))
        fail_at("Symbol section " + std::string(name) + " expects an identifier atom.", value.source_offset);
    return result;
}

std::size_t symbol_section_value_offset(const Node& section) { return section.children.size() == 2 ? section.children[1].atom_offset : section.source_offset; }

std::string identifier_atom(const Node& node, std::string_view context)
{
    if (node.list || node.quoted)
        fail_at(std::string(context) + " expects an unquoted atom.", node.source_offset);

    const auto result = atom(node, context);
    if (!is_symbol_atom(result))
        fail_at(std::string(context) + " expects an identifier atom.", node.source_offset);
    return result;
}

std::string optional_symbol_value(const Node& node, std::string_view name)
{
    if (const auto* section = find_section(node, name))
        return parse_symbol_section_value(*section, name);
    return {};
}

const Node* optional_single_value_section(const Node& node, std::string_view name)
{
    if (const auto* section = find_section(node, name))
    {
        if (section->children.size() != 2)
            fail_at("Malformed rule section " + std::string(name) + ".", section->source_offset);
        return &section->children[1];
    }
    return nullptr;
}

ast::Feature parse_feature(const Node& node)
{
    if (!node.list || node.children.size() < 2)
        fail_at("Malformed feature.", node.source_offset);

    validate_feature_sections(node);

    const auto& symbol_section = require_section(node, ":symbol");
    const auto symbol = parse_symbol_section_value(symbol_section, ":symbol");

    const auto* expression = optional_single_value_section(node, ":expression");
    if (expression == nullptr)
        fail_at("Feature is missing :expression section.", node.source_offset);

    return ast::Feature { node.source_offset,
                          symbol_section_value_offset(symbol_section),
                          expression->source_offset,
                          parse_feature_kind(node.children[0]),
                          symbol,
                          symbol,
                          optional_metadata_value(node, ":description"),
                          render(*expression) };
}

bool is_allowed_transition_section(std::string_view name) { return name == ":symbol" || name == ":description" || name == ":expression"; }

void validate_transition_sections(const Node& transition)
{
    auto seen = std::unordered_set<std::string> {};
    for (std::size_t i = 1; i < transition.children.size(); ++i)
    {
        const auto& section = transition.children[i];
        if (!section.list || section.children.empty())
            fail_at("Malformed transition section.", section.source_offset);

        const auto name = atom(section.children[0], "transition section");
        if (!is_allowed_transition_section(name))
            fail_at("Transition section " + name + " is not valid.", section.source_offset);
        if (!seen.emplace(name).second)
            fail_at("Duplicate transition section " + name + ".", section.source_offset);
    }
}

ast::MemoryTransition parse_transition(const Node& node)
{
    if (!node.list || node.children.empty() || atom(node.children[0], "rule") != ":rule")
        fail_at("Malformed memory rule.", node.source_offset);

    validate_transition_sections(node);
    const auto& expression = require_section(node, ":expression");
    const auto& source_node = require_single_value_section_value(expression, ":source-memory");
    const auto& target_node = require_single_value_section_value(expression, ":target-memory");

    auto result = ast::MemoryTransition { node.source_offset,
                                          source_node.atom_offset,
                                          target_node.atom_offset,
                                          identifier_atom(source_node, ":source-memory"),
                                          identifier_atom(target_node, ":target-memory"),
                                          optional_symbol_value(node, ":symbol"),
                                          optional_metadata_value(node, ":description"),
                                          {} };
    for (std::size_t i = 1; i < expression.children.size(); ++i)
    {
        const auto& child = expression.children[i];
        const auto child_head = head(child, "transition expression");
        if (child_head == ":source-memory" || child_head == ":target-memory")
            continue;
        result.rules.push_back(parse_rule(child));
    }
    return result;
}

struct ModuleSectionPresence
{
    bool symbol = false;
    bool description = false;
    bool arguments = false;
    bool registers = false;
    bool entry = false;
    bool memory = false;
    bool features = false;
    bool transitions = false;
};

void require_new_section(bool& seen, std::string_view name, std::size_t offset)
{
    if (seen)
        fail_at("Duplicate module section " + std::string(name) + ".", offset);
    seen = true;
}

void parse_module_section(ast::Module& result, const Node& section, ModuleSectionPresence& presence)
{
    const auto name = head(section, "module section");
    if (name == ":symbol")
    {
        require_new_section(presence.symbol, name, section.source_offset);
        result.name = parse_symbol_section_value(section, ":symbol");
        result.source_offset = symbol_section_value_offset(section);
    }
    else if (name == ":description")
    {
        require_new_section(presence.description, name, section.source_offset);
        if (section.children.size() != 2)
            fail_at("Malformed metadata section :description.", section.source_offset);
        static_cast<void>(atom(section.children[1], ":description"));
    }
    else if (name == ":arguments")
    {
        require_new_section(presence.arguments, name, section.source_offset);
        for (std::size_t i = 1; i < section.children.size(); ++i)
            result.arguments.push_back(parse_argument(section.children[i]));
    }
    else if (name == ":registers")
    {
        require_new_section(presence.registers, name, section.source_offset);
        for (std::size_t i = 1; i < section.children.size(); ++i)
            result.registers.push_back(parse_register(section.children[i]));
    }
    else if (name == ":entry")
    {
        require_new_section(presence.entry, name, section.source_offset);
        if (section.children.size() != 2)
            fail_at("Malformed entry section.", section.source_offset);
        result.entry = identifier_atom(section.children[1], ":entry");
        result.entry_offset = section.children[1].atom_offset;
    }
    else if (name == ":memory")
    {
        require_new_section(presence.memory, name, section.source_offset);
        if (section.children.size() < 2)
            fail_at("Module :memory section must declare at least one memory state.", section.source_offset);
        for (std::size_t i = 1; i < section.children.size(); ++i)
            result.memory_states.push_back(ast::NamedValue { section.children[i].atom_offset, identifier_atom(section.children[i], ":memory") });
    }
    else if (name == ":features")
    {
        require_new_section(presence.features, name, section.source_offset);
        for (std::size_t i = 1; i < section.children.size(); ++i)
            result.features.push_back(parse_feature(section.children[i]));
    }
    else if (name == ":rules")
    {
        require_new_section(presence.transitions, name, section.source_offset);
        for (std::size_t i = 1; i < section.children.size(); ++i)
            result.transitions.push_back(parse_transition(section.children[i]));
    }
    else
    {
        fail_at("Unknown module section: " + name + ".", section.source_offset);
    }
}

void require_module_section(bool seen, std::string_view name, std::size_t offset)
{
    if (!seen)
        fail_at("Module is missing section " + std::string(name) + ".", offset);
}

ast::Module parse_module_node(const Node& root)
{
    if (!root.list || root.children.size() < 2 || atom(root.children[0], "module") != ":module")
        fail_at("Expected module description.", root.source_offset);

    auto result = ast::Module {};
    result.source_offset = root.source_offset;
    auto presence = ModuleSectionPresence {};
    auto first_section = std::size_t { 1 };
    if (!root.children[1].list)
    {
        result.name = atom(root.children[1], "module name");
        first_section = 2;
    }
    for (std::size_t i = first_section; i < root.children.size(); ++i)
        parse_module_section(result, root.children[i], presence);

    if (result.name.empty())
        fail_at("Module is missing section :symbol.", root.source_offset);
    require_module_section(presence.arguments, ":arguments", root.source_offset);
    require_module_section(presence.registers, ":registers", root.source_offset);
    require_module_section(presence.entry, ":entry", root.source_offset);
    require_module_section(presence.memory, ":memory", root.source_offset);
    require_module_section(presence.features, ":features", root.source_offset);
    require_module_section(presence.transitions, ":rules", root.source_offset);
    return result;
}

void parse_program_section(ast::ModuleProgram& result, const Node& section)
{
    if (!section.list || section.children.empty())
        fail_at("Malformed program section.", section.source_offset);

    const auto section_head = head(section, "program section");
    if (section_head == ":entry")
    {
        if (!result.entry.empty())
            fail_at("Duplicate program entry section.", section.source_offset);
        if (section.children.size() != 2)
            fail_at("Malformed program entry section.", section.source_offset);
        result.entry = atom(section.children[1], ":entry");
        result.entry_offset = section.children[1].atom_offset;
    }
    else if (section_head == ":module")
    {
        result.modules.push_back(parse_module_node(section));
    }
    else
    {
        fail_at("Unknown program section: " + section_head + ".", section.source_offset);
    }
}

}  // namespace

ast::Module parse_module_ast(const std::string& description)
{
    validate_s_expression_syntax_with_x3(description);
    return parse_module_node(Reader(description).read_root());
}

ast::ModuleProgram parse_module_program_ast(const std::string& description)
{
    validate_s_expression_syntax_with_x3(description);
    const auto root = Reader(description).read_root();
    if (!root.list || root.children.empty() || atom(root.children[0], "program") != ":program")
        fail_at("Expected module program description.", root.source_offset);

    auto result = ast::ModuleProgram {};
    result.source_offset = root.source_offset;
    for (std::size_t i = 1; i < root.children.size(); ++i)
        parse_program_section(result, root.children[i]);

    if (result.entry.empty())
        fail_at("Module program is missing an entry module.", root.source_offset);
    if (result.modules.empty())
        fail_at("Module program must contain at least one module.", root.source_offset);
    return result;
}

}  // namespace runir::kr::ps::ext::dl::parser
