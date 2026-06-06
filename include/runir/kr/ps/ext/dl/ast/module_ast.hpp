#ifndef RUNIR_KR_PS_EXT_DL_AST_MODULE_AST_HPP_
#define RUNIR_KR_PS_EXT_DL_AST_MODULE_AST_HPP_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace runir::kr::ps::ext::dl::ast
{

enum class ArgumentKind
{
    CONCEPT,
    ROLE,
    BOOLEAN,
    NUMERICAL,
};

enum class FeatureKind
{
    CONCEPT,
    BOOLEAN,
    NUMERICAL,
};

enum class ObservationKind
{
    POSITIVE,
    NEGATIVE,
    EQUAL_ZERO,
    GREATER_ZERO,
    UNCHANGED,
    INCREASES,
    DECREASES,
};

enum class RuleKind
{
    LOAD,
    SKETCH,
    DO,
    CALL,
};

struct Argument
{
    std::size_t source_offset = 0;
    std::size_t name_offset = 0;
    ArgumentKind kind;
    std::string name;
    std::uint64_t identifier = 0;
};

struct Register
{
    std::size_t source_offset = 0;
    std::size_t name_offset = 0;
    std::string name;
    std::uint64_t identifier = 0;
};

struct Feature
{
    std::size_t source_offset = 0;
    std::size_t name_offset = 0;
    std::size_t expression_offset = 0;
    FeatureKind kind;
    std::string name;
    std::string symbol;
    std::string description;
    std::string expression;
};

struct Observation
{
    std::size_t source_offset = 0;
    std::size_t feature_offset = 0;
    ObservationKind kind;
    std::string feature;
};

struct Expression
{
    std::size_t source_offset = 0;
    std::string text;
};

struct NamedValue
{
    std::size_t source_offset = 0;
    std::string value;
};

struct Rule
{
    RuleKind kind;
    std::size_t source_offset = 0;
    std::size_t concept_expression_offset = 0;
    std::size_t register_offset = 0;
    std::size_t action_offset = 0;
    std::size_t callee_offset = 0;
    std::size_t arguments_offset = 0;
    std::string symbol;
    std::string description;
    std::vector<Observation> conditions;
    std::vector<Observation> effects;
    std::string concept_expression;
    std::string reg;
    std::string action;
    std::string callee;
    std::vector<Expression> arguments;
};

struct MemoryTransition
{
    std::size_t source_offset = 0;
    std::size_t source_name_offset = 0;
    std::size_t target_name_offset = 0;
    std::string source;
    std::string target;
    std::string symbol;
    std::string description;
    std::vector<Rule> rules;
};

struct Module
{
    std::size_t source_offset = 0;
    std::string name;
    std::vector<Argument> arguments;
    std::vector<Register> registers;
    std::size_t entry_offset = 0;
    std::string entry;
    std::vector<NamedValue> memory_states;
    std::vector<Feature> features;
    std::vector<MemoryTransition> transitions;
};

struct ModuleProgram
{
    std::size_t source_offset = 0;
    std::size_t entry_offset = 0;
    std::string entry;
    std::vector<Module> modules;
};

}  // namespace runir::kr::ps::ext::dl::ast

#endif
