#include "runir/kr/ps/ext/dl/parser/parser.hpp"

#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext::dl::parser
{
namespace
{

struct Node
{
    bool list = false;
    bool quoted = false;
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
        ++m_pos;
        auto result = Node {};
        result.list = true;
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
        ++m_pos;
        auto result = Node {};
        result.quoted = true;
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
        auto result = Node {};
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

[[noreturn]] void fail(const std::string& message) { throw std::runtime_error(message); }

const Node& require_child(const Node& node, std::size_t index, std::string_view context)
{
    if (!node.list || index >= node.children.size())
        fail(std::string("Malformed ") + std::string(context) + ".");
    return node.children[index];
}

std::string atom(const Node& node, std::string_view context)
{
    if (node.list)
        fail(std::string("Expected atom in ") + std::string(context) + ".");
    return node.atom;
}

std::string head(const Node& node, std::string_view context) { return atom(require_child(node, 0, context), context); }

std::uint64_t integer(const Node& node, std::string_view context)
{
    const auto text = atom(node, context);
    std::size_t read = 0;
    const auto result = std::stoull(text, &read);
    if (read != text.size())
        fail(std::string("Expected integer in ") + std::string(context) + ".");
    return result;
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

ast::ArgumentKind parse_argument_kind(const std::string& value)
{
    if (value == "concept")
        return ast::ArgumentKind::CONCEPT;
    if (value == "role")
        return ast::ArgumentKind::ROLE;
    if (value == "boolean")
        return ast::ArgumentKind::BOOLEAN;
    if (value == "numerical")
        return ast::ArgumentKind::NUMERICAL;
    fail("Unknown argument kind: " + value + ".");
}

ast::FeatureKind parse_feature_kind(const std::string& value)
{
    if (value == "concept")
        return ast::FeatureKind::CONCEPT;
    if (value == "boolean")
        return ast::FeatureKind::BOOLEAN;
    if (value == "numerical")
        return ast::FeatureKind::NUMERICAL;
    fail("Unknown feature kind: " + value + ".");
}

ast::ObservationKind parse_observation_kind(const std::string& value)
{
    if (value == "positive")
        return ast::ObservationKind::POSITIVE;
    if (value == "negative")
        return ast::ObservationKind::NEGATIVE;
    if (value == "equal_zero")
        return ast::ObservationKind::EQUAL_ZERO;
    if (value == "greater_zero")
        return ast::ObservationKind::GREATER_ZERO;
    if (value == "unchanged")
        return ast::ObservationKind::UNCHANGED;
    if (value == "increases")
        return ast::ObservationKind::INCREASES;
    if (value == "decreases")
        return ast::ObservationKind::DECREASES;
    fail("Unknown observation kind: " + value + ".");
}

ast::RuleKind parse_rule_kind(const std::string& value)
{
    if (value == "load")
        return ast::RuleKind::LOAD;
    if (value == "sketch")
        return ast::RuleKind::SKETCH;
    if (value == "do")
        return ast::RuleKind::DO;
    if (value == "call")
        return ast::RuleKind::CALL;
    fail("Unknown rule kind: " + value + ".");
}

ast::Observation parse_observation(const Node& node)
{
    if (!node.list || node.children.size() != 2)
        fail("Malformed observation.");
    return ast::Observation { parse_observation_kind(atom(node.children[0], "observation")), render(node.children[1]) };
}

std::vector<ast::Observation> parse_observation_section(const Node& section, std::string_view name)
{
    if (head(section, name) != name)
        fail(std::string("Expected section ") + std::string(name) + ".");
    auto result = std::vector<ast::Observation> {};
    for (std::size_t i = 1; i < section.children.size(); ++i)
        result.push_back(parse_observation(section.children[i]));
    return result;
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
    fail(std::string("Missing rule section ") + std::string(name) + ".");
}

std::vector<std::string> parse_expression_list_section(const Node& rule, std::string_view name)
{
    const auto& section = require_section(rule, name);
    auto result = std::vector<std::string> {};
    for (std::size_t i = 1; i < section.children.size(); ++i)
        result.push_back(render(section.children[i]));
    return result;
}

std::string parse_single_value_section(const Node& rule, std::string_view name)
{
    const auto& section = require_section(rule, name);
    if (section.children.size() != 2)
        fail(std::string("Malformed rule section ") + std::string(name) + ".");
    return atom(section.children[1], name);
}

std::string parse_single_expression_section(const Node& rule, std::string_view name)
{
    const auto& section = require_section(rule, name);
    if (section.children.size() != 2)
        fail(std::string("Malformed rule section ") + std::string(name) + ".");
    return render(section.children[1]);
}

ast::Rule parse_rule(const Node& node)
{
    if (!node.list || node.children.empty())
        fail("Malformed rule.");
    auto result = ast::Rule {};
    result.kind = parse_rule_kind(head(node, "rule"));
    result.conditions = parse_observation_section(require_section(node, ":conditions"), ":conditions");

    switch (result.kind)
    {
        case ast::RuleKind::LOAD:
            result.concept_expression = parse_single_expression_section(node, ":concept");
            result.reg = parse_single_value_section(node, ":register");
            break;
        case ast::RuleKind::SKETCH:
            result.effects = parse_observation_section(require_section(node, ":effects"), ":effects");
            break;
        case ast::RuleKind::DO:
            result.action = parse_single_value_section(node, ":action");
            result.arguments = parse_expression_list_section(node, ":arguments");
            break;
        case ast::RuleKind::CALL:
            result.callee = parse_single_value_section(node, ":callee");
            result.arguments = parse_expression_list_section(node, ":arguments");
            break;
    }

    return result;
}

ast::Argument parse_argument(const Node& node)
{
    if (!node.list || node.children.size() != 3)
        fail("Malformed argument.");
    return ast::Argument { parse_argument_kind(atom(node.children[0], "argument")), atom(node.children[1], "argument"), integer(node.children[2], "argument") };
}

ast::Register parse_register(const Node& node)
{
    if (!node.list || node.children.size() != 3 || atom(node.children[0], "register") != "concept")
        fail("Malformed register.");
    return ast::Register { atom(node.children[1], "register"), integer(node.children[2], "register") };
}

ast::Feature parse_feature(const Node& node)
{
    if (!node.list || node.children.size() != 5)
        fail("Malformed feature.");
    return ast::Feature { parse_feature_kind(atom(node.children[0], "feature")),
                          atom(node.children[1], "feature"),
                          atom(node.children[2], "feature"),
                          atom(node.children[3], "feature"),
                          render(node.children[4]) };
}

ast::MemoryTransition parse_transition(const Node& node)
{
    if (!node.list || node.children.size() < 3)
        fail("Malformed memory transition.");
    auto result = ast::MemoryTransition { atom(node.children[0], "transition"), atom(node.children[1], "transition"), {} };
    for (std::size_t i = 2; i < node.children.size(); ++i)
        result.rules.push_back(parse_rule(node.children[i]));
    return result;
}

void parse_module_section(ast::Module& result, const Node& section)
{
    const auto name = head(section, "module section");
    if (name == ":arguments")
    {
        for (std::size_t i = 1; i < section.children.size(); ++i)
            result.arguments.push_back(parse_argument(section.children[i]));
    }
    else if (name == ":registers")
    {
        for (std::size_t i = 1; i < section.children.size(); ++i)
            result.registers.push_back(parse_register(section.children[i]));
    }
    else if (name == ":entry")
    {
        if (section.children.size() != 2)
            fail("Malformed entry section.");
        result.entry = atom(section.children[1], ":entry");
    }
    else if (name == ":memory")
    {
        for (std::size_t i = 1; i < section.children.size(); ++i)
            result.memory_states.push_back(atom(section.children[i], ":memory"));
    }
    else if (name == ":features")
    {
        for (std::size_t i = 1; i < section.children.size(); ++i)
            result.features.push_back(parse_feature(section.children[i]));
    }
    else if (name == ":transitions")
    {
        for (std::size_t i = 1; i < section.children.size(); ++i)
            result.transitions.push_back(parse_transition(section.children[i]));
    }
    else
    {
        fail("Unknown module section: " + name + ".");
    }
}

ast::Module parse_module_node(const Node& root)
{
    if (!root.list || root.children.size() < 2 || atom(root.children[0], "module") != "module")
        fail("Expected module description.");

    auto result = ast::Module {};
    result.name = atom(root.children[1], "module name");
    for (std::size_t i = 2; i < root.children.size(); ++i)
        parse_module_section(result, root.children[i]);

    if (result.entry.empty())
        fail("Module is missing an entry memory state.");
    return result;
}

void parse_program_section(ast::ModuleProgram& result, const Node& section)
{
    if (!section.list || section.children.empty())
        fail("Malformed program section.");

    const auto section_head = head(section, "program section");
    if (section_head == ":entry")
    {
        if (section.children.size() != 2)
            fail("Malformed program entry section.");
        result.entry = atom(section.children[1], ":entry");
    }
    else if (section_head == "module")
    {
        result.modules.push_back(parse_module_node(section));
    }
    else
    {
        fail("Unknown program section: " + section_head + ".");
    }
}

}  // namespace

ast::Module parse_module_ast(const std::string& description) { return parse_module_node(Reader(description).read_root()); }

ast::ModuleProgram parse_module_program_ast(const std::string& description)
{
    const auto root = Reader(description).read_root();
    if (!root.list || root.children.empty() || atom(root.children[0], "program") != "program")
        fail("Expected module program description.");

    auto result = ast::ModuleProgram {};
    for (std::size_t i = 1; i < root.children.size(); ++i)
        parse_program_section(result, root.children[i]);

    if (result.entry.empty())
        fail("Module program is missing an entry module.");
    if (result.modules.empty())
        fail("Module program must contain at least one module.");
    return result;
}

}  // namespace runir::kr::ps::ext::dl::parser
