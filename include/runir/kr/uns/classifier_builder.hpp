#ifndef RUNIR_KR_UNS_CLASSIFIER_BUILDER_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_BUILDER_HPP_

#include "runir/kr/uns/classifier_data.hpp"
#include "runir/kr/uns/classifier_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of ClassifierLiteral: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::uns::ClassifierLiteral, C>
{
    urd::index<runir::kr::uns::ClassifierLiteral> index;
    urd::built_t<runir::kr::uns::ClassifierLiteralVariant, C> feature;
    urd::built_t<bool, C> polarity = true;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"feature", &builder::feature>, urd::field<"polarity", &builder::polarity>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::feature, &builder::polarity> {}; }
};

/// The builder of ClassifierClause: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::uns::ClassifierClause, C>
{
    urd::index<runir::kr::uns::ClassifierClause> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::uns::ClassifierLiteral>>, C> literals;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"literals", &builder::literals>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::literals> {}; }
};

/// The builder of Classifier: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::uns::Classifier, C>
{
    urd::index<runir::kr::uns::Classifier> index;
    urd::built_t<urd::flat_string, C> symbol;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>>, C> features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::uns::ClassifierClause>>, C> clauses;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"symbol", &builder::symbol>, urd::field<"features", &builder::features>, urd::field<"clauses", &builder::clauses>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::symbol, &builder::features, &builder::clauses> {}; }
};

}  // namespace urd

#endif
