#ifndef RUNIR_SERIALIZATION_KR_UNS_CLASSIFIER_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_UNS_CLASSIFIER_VIEW_HPP_

#include "runir/kr/uns/classifier_view.hpp"
#include "runir/serialization/kr/ps/feature_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename C>
struct TypeName<View<Index<runir::kr::uns::ClassifierLiteral>, C>>
{
    static std::string get() { return "Uns.ClassifierLiteral"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::uns::ClassifierLiteral>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        using Feature = Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>>;
        ar.field("feature", value.get_feature().template get<Feature>());
        ar.field("polarity", value.get_polarity());
    });
}

template<typename C>
struct TypeName<View<Index<runir::kr::uns::ClassifierClause>, C>>
{
    static std::string get() { return "Uns.ClassifierClause"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::uns::ClassifierClause>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("literals", value.get_literals());
    });
}

template<typename C>
struct TypeName<View<Index<runir::kr::uns::Classifier>, C>>
{
    static std::string get() { return "Uns.Classifier"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::uns::Classifier>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("symbol", value.get_symbol());
        ar.field("features", value.get_features());
        ar.field("clauses", value.get_clauses());
    });
}

}

#endif
