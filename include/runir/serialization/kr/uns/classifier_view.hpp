#ifndef RUNIR_SERIALIZATION_KR_UNS_CLASSIFIER_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_UNS_CLASSIFIER_VIEW_HPP_

#include "runir/kr/uns/classifier_view.hpp"
#include "runir/serialization/kr/ps/feature_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::uns::ClassifierLiteral>, C>>)
{
    using Feature = Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>>;
    ar.field("feature", [](const auto& value) -> decltype(auto) { return (value.get_feature().template get<Feature>()); });
    ar.field("polarity", [](const auto& value) -> decltype(auto) { return (value.get_polarity()); });
}

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::uns::ClassifierClause>, C>>)
{
    ar.field("literals", [](const auto& value) -> decltype(auto) { return (value.get_literals()); });
}

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::uns::Classifier>, C>>)
{
    ar.field("symbol", [](const auto& value) -> decltype(auto) { return (value.get_symbol()); });
    ar.field("features", [](const auto& value) -> decltype(auto) { return (value.get_features()); });
    ar.field("clauses", [](const auto& value) -> decltype(auto) { return (value.get_clauses()); });
}

}

#endif
