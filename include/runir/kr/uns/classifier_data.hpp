#ifndef RUNIR_KR_UNS_CLASSIFIER_DATA_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/uns/declarations.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include <urd/elements/flat.hpp>

namespace urd
{

// A literal: a boolean feature (wrapped in a single-alternative variant, mirroring the kr/ps condition
// layering) together with a polarity (true = the feature must hold, false = negated).
template<>
struct data<runir::kr::uns::ClassifierLiteral>
{
    using Variant = urd::flat_variant<urd::index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>>;

    urd::index<runir::kr::uns::ClassifierLiteral> index;
    Variant variant;
    bool polarity = true;

    data() = default;
    data(Variant variant_, bool polarity_) : index(), variant(std::move(variant_)), polarity(polarity_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::variant, &data::polarity> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::variant, &data::polarity> {}; }
};

// A clause: a conjunction of literals.
template<>
struct data<runir::kr::uns::ClassifierClause>
{
    urd::index<runir::kr::uns::ClassifierClause> index;
    urd::flat_vector<urd::index<runir::kr::uns::ClassifierLiteral>> literals;

    data() = default;
    data(urd::flat_vector<urd::index<runir::kr::uns::ClassifierLiteral>> literals_) : index(), literals(std::move(literals_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::literals> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::literals> {}; }
};

// A classifier: named boolean features + a DNF (disjunction of clauses).
template<>
struct data<runir::kr::uns::Classifier>
{
    urd::index<runir::kr::uns::Classifier> index;
    urd::flat_string symbol;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>> features;
    urd::flat_vector<urd::index<runir::kr::uns::ClassifierClause>> clauses;

    data() = default;
    data(urd::flat_string symbol_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>> features_,
         urd::flat_vector<urd::index<runir::kr::uns::ClassifierClause>> clauses_) :
        index(),
        symbol(std::move(symbol_)),
        features(std::move(features_)),
        clauses(std::move(clauses_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::symbol, &data::features, &data::clauses> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::symbol, &data::features, &data::clauses> {}; }
};

}  // namespace urd

#endif
