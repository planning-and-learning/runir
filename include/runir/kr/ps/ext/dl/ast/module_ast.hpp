#ifndef RUNIR_KR_PS_EXT_DL_AST_MODULE_AST_HPP_
#define RUNIR_KR_PS_EXT_DL_AST_MODULE_AST_HPP_

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
    ArgumentKind kind;
    std::string name;
    std::uint64_t identifier = 0;
};

struct Register
{
    std::string name;
    std::uint64_t identifier = 0;
};

struct Feature
{
    FeatureKind kind;
    std::string name;
    std::string symbol;
    std::string description;
    std::string expression;
};

struct Observation
{
    ObservationKind kind;
    std::string feature;
};

struct Rule
{
    RuleKind kind;
    std::vector<Observation> conditions;
    std::vector<Observation> effects;
    std::string concept_expression;
    std::string reg;
    std::string action;
    std::string callee;
    std::vector<std::string> arguments;
};

struct MemoryTransition
{
    std::string source;
    std::string target;
    std::vector<Rule> rules;
};

struct Module
{
    std::string name;
    std::vector<Argument> arguments;
    std::vector<Register> registers;
    std::string entry;
    std::vector<std::string> memory_states;
    std::vector<Feature> features;
    std::vector<MemoryTransition> transitions;
};

struct ModuleProgram
{
    std::string entry;
    std::vector<Module> modules;
};

}  // namespace runir::kr::ps::ext::dl::ast

#endif
