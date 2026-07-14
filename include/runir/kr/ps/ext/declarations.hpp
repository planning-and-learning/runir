#ifndef RUNIR_KR_PS_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <concepts>

namespace runir::kr::ps::ext
{

struct MemoryState
{
};

struct ModuleSymbol
{
};

// Rules

struct SketchTag
{
    static constexpr auto keyword = "sketch";
};

template<runir::kr::dl::CategoryTag CategoryT>
struct LoadTag
{
    static constexpr auto keyword = "load";
};

struct DoTag
{
    static constexpr auto keyword = "do";
};

struct CallTag
{
    static constexpr auto keyword = "call";
};

template<typename T>
concept RuleKind = std::same_as<T, LoadTag<runir::kr::dl::ConceptTag>> || std::same_as<T, LoadTag<runir::kr::dl::RoleTag>> || std::same_as<T, SketchTag>
                   || std::same_as<T, DoTag> || std::same_as<T, CallTag>;

template<RuleKind Kind>
struct Rule
{
};

struct RuleVariant
{
};

struct Module
{
};

struct ModuleProgram
{
};

}  // namespace runir::kr::ps::ext

#endif
