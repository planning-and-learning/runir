#ifndef RUNIR_KR_PS_BASE_DL_AST_AST_HPP_
#define RUNIR_KR_PS_BASE_DL_AST_AST_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <string>
#include <vector>

namespace runir::kr::ps::base::dl::ast
{
namespace x3 = boost::spirit::x3;

template<typename... Alternatives>
struct PositionedVariant : x3::position_tagged, x3::variant<Alternatives...>
{
    using Base = x3::variant<Alternatives...>;
    using Base::Base;
    using Base::operator=;
};

struct Positive : x3::position_tagged
{
    static constexpr auto keyword = "positive";
};

struct Negative : x3::position_tagged
{
    static constexpr auto keyword = "negative";
};

struct EqualZero : x3::position_tagged
{
    static constexpr auto keyword = "equal_zero";
};

struct GreaterZero : x3::position_tagged
{
    static constexpr auto keyword = "greater_zero";
};

struct Increases : x3::position_tagged
{
    static constexpr auto keyword = "increases";
};

struct Decreases : x3::position_tagged
{
    static constexpr auto keyword = "decreases";
};

struct Unchanged : x3::position_tagged
{
    static constexpr auto keyword = "unchanged";
};

template<runir::kr::FamilyTag Family>
struct Concept : x3::position_tagged
{
    std::string name;
    std::string symbol;
    std::string description;
    runir::kr::dl::grammar::ast::Constructor<Family, runir::kr::dl::ConceptTag> feature;
};

template<runir::kr::FamilyTag Family>
struct BooleanFeature : x3::position_tagged
{
    std::string name;
    std::string symbol;
    std::string description;
    runir::kr::dl::grammar::ast::Constructor<Family, runir::kr::dl::BooleanTag> feature;
};

template<runir::kr::FamilyTag Family>
struct NumericalFeature : x3::position_tagged
{
    std::string name;
    std::string symbol;
    std::string description;
    runir::kr::dl::grammar::ast::Constructor<Family, runir::kr::dl::NumericalTag> feature;
};

template<runir::kr::FamilyTag Family>
struct Feature : PositionedVariant<BooleanFeature<Family>, NumericalFeature<Family>>
{
    using Base = PositionedVariant<BooleanFeature<Family>, NumericalFeature<Family>>;
    using Base::Base;
    using Base::operator=;
};

template<>
struct Feature<runir::kr::ExtFamilyTag> :
    PositionedVariant<Concept<runir::kr::ExtFamilyTag>, BooleanFeature<runir::kr::ExtFamilyTag>, NumericalFeature<runir::kr::ExtFamilyTag>>
{
    using Base = PositionedVariant<Concept<runir::kr::ExtFamilyTag>, BooleanFeature<runir::kr::ExtFamilyTag>, NumericalFeature<runir::kr::ExtFamilyTag>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::FamilyTag Family>
struct ConditionObservation : PositionedVariant<Positive, Negative, EqualZero, GreaterZero>
{
    using Base = PositionedVariant<Positive, Negative, EqualZero, GreaterZero>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::FamilyTag Family>
struct Condition : x3::position_tagged
{
    ConditionObservation<Family> observation;
    std::string feature;
};

template<runir::kr::FamilyTag Family>
struct EffectObservation : PositionedVariant<Positive, Negative, Unchanged, Increases, Decreases>
{
    using Base = PositionedVariant<Positive, Negative, Unchanged, Increases, Decreases>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::FamilyTag Family>
struct Effect : x3::position_tagged
{
    EffectObservation<Family> observation;
    std::string feature;
};

template<runir::kr::FamilyTag Family>
struct Rule : x3::position_tagged
{
    std::vector<Condition<Family>> conditions;
    std::vector<Effect<Family>> effects;
};

template<runir::kr::FamilyTag Family>
struct Sketch : x3::position_tagged
{
    std::vector<Feature<Family>> features;
    std::vector<Rule<Family>> rules;
};

using BaseConcept = Concept<runir::kr::BaseFamilyTag>;
using BaseBooleanFeature = BooleanFeature<runir::kr::BaseFamilyTag>;
using BaseNumericalFeature = NumericalFeature<runir::kr::BaseFamilyTag>;
using BaseFeature = Feature<runir::kr::BaseFamilyTag>;
using BaseConditionObservation = ConditionObservation<runir::kr::BaseFamilyTag>;
using BaseCondition = Condition<runir::kr::BaseFamilyTag>;
using BaseEffectObservation = EffectObservation<runir::kr::BaseFamilyTag>;
using BaseEffect = Effect<runir::kr::BaseFamilyTag>;
using BaseRule = Rule<runir::kr::BaseFamilyTag>;
using BaseSketch = Sketch<runir::kr::BaseFamilyTag>;

using ExtConcept = Concept<runir::kr::ExtFamilyTag>;
using ExtBooleanFeature = BooleanFeature<runir::kr::ExtFamilyTag>;
using ExtNumericalFeature = NumericalFeature<runir::kr::ExtFamilyTag>;
using ExtFeature = Feature<runir::kr::ExtFamilyTag>;
using ExtConditionObservation = ConditionObservation<runir::kr::ExtFamilyTag>;
using ExtCondition = Condition<runir::kr::ExtFamilyTag>;
using ExtEffectObservation = EffectObservation<runir::kr::ExtFamilyTag>;
using ExtEffect = Effect<runir::kr::ExtFamilyTag>;
using ExtRule = Rule<runir::kr::ExtFamilyTag>;
using ExtSketch = Sketch<runir::kr::ExtFamilyTag>;

}  // namespace runir::kr::ps::base::dl::ast

#endif
