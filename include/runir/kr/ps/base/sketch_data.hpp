#ifndef RUNIR_KR_PS_BASE_SKETCH_DATA_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_DATA_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_index.hpp"
#include "runir/kr/ps/base/sketch_index.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/feature_index.hpp"

#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::base::Sketch>
{
    Index<runir::kr::ps::base::Sketch> index;
    IndexList<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>> boolean_features;
    IndexList<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>> numerical_features;
    IndexList<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>> rules;

    Data() = default;
    Data(IndexList<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>> boolean_features_,
         IndexList<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>> numerical_features_,
         IndexList<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>> rules_) :
        index(),
        boolean_features(std::move(boolean_features_)),
        numerical_features(std::move(numerical_features_)),
        rules(std::move(rules_))
    {
    }
    template<typename C>
    Data(const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>, C>>& boolean_features_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>, C>>& numerical_features_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>, C>>& rules_) :
        index(),
        boolean_features(),
        numerical_features(),
        rules()
    {
        set(boolean_features_, boolean_features);
        set(numerical_features_, numerical_features);
        set(rules_, rules);
    }

    auto cista_members() noexcept { return std::tie(index, boolean_features, numerical_features, rules); }
    auto cista_members() const noexcept { return std::tie(index, boolean_features, numerical_features, rules); }
    auto identifying_members() const noexcept { return std::tie(boolean_features, numerical_features, rules); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
