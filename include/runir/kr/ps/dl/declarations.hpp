#ifndef RUNIR_KR_PS_DL_DECLARATIONS_HPP_
#define RUNIR_KR_PS_DL_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <concepts>

namespace runir::kr::ps::dl
{

struct QueryFeature
{
    static constexpr auto keyword = "query";
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

struct Unconstrained
{
    static constexpr auto keyword = "unconstrained";
};

// Features use the DL categories as tags; a query feature has no DL category of its own.
// These are semantic categories, independent of a default policy repository's inventory.
template<typename T>
concept FeatureTag = std::same_as<T, runir::kr::dl::ConceptTag> || std::same_as<T, runir::kr::dl::RoleTag> || std::same_as<T, runir::kr::dl::BooleanTag>
                     || std::same_as<T, runir::kr::dl::NumericalTag> || std::same_as<T, QueryFeature>;

template<typename Observation, typename Feature>
concept ConditionObservationTag =
    (std::same_as<Feature, runir::kr::dl::BooleanTag> && (std::same_as<Observation, Positive> || std::same_as<Observation, Negative>) )
    || (std::same_as<Feature, runir::kr::dl::NumericalTag> && (std::same_as<Observation, EqualZero> || std::same_as<Observation, GreaterZero>) );

template<typename Observation, typename Feature>
concept EffectObservationTag = (std::same_as<Feature, runir::kr::dl::BooleanTag>
                                && (std::same_as<Observation, Positive> || std::same_as<Observation, Negative> || std::same_as<Observation, Unchanged>) )
                               || (std::same_as<Feature, runir::kr::dl::NumericalTag>
                                   && (std::same_as<Observation, Increases> || std::same_as<Observation, Decreases> || std::same_as<Observation, Unchanged>) );

}  // namespace runir::kr::ps::dl

#endif
