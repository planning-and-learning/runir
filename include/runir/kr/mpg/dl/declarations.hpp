#ifndef RUNIR_KR_MPG_DL_DECLARATIONS_HPP_
#define RUNIR_KR_MPG_DL_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/mpg/declarations.hpp"

namespace runir::kr::mpg::dl
{

struct RegisterTag
{
    static constexpr auto keyword = "register";
};

template<typename T>
concept ConceptConstructorTag = runir::kr::dl::ConceptConstructorTag<T> || std::same_as<T, RegisterTag>;

template<ConceptConstructorTag Tag>
struct Concept;

template<runir::kr::dl::CategoryTag Category>
struct Constructor;

struct ConceptFeature
{
    static constexpr auto keyword = "concept";
};

struct BooleanFeature
{
    static constexpr auto keyword = "boolean";
};

struct NumericalFeature
{
    static constexpr auto keyword = "numerical";
};

struct Positive
{
    static constexpr auto keyword = "positive";
};

struct Negative
{
    static constexpr auto keyword = "negative";
};

struct EqualZero
{
    static constexpr auto keyword = "equal_zero";
};

struct GreaterZero
{
    static constexpr auto keyword = "greater_zero";
};

struct Increases
{
    static constexpr auto keyword = "increases";
};

struct Decreases
{
    static constexpr auto keyword = "decreases";
};

struct Unchanged
{
    static constexpr auto keyword = "unchanged";
};

}  // namespace runir::kr::mpg::dl

#endif
