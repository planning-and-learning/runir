#ifndef RUNIR_KR_UNS_CLASSIFIER_DATA_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace runir::kr::uns
{

/// The boolean feature of a classifier literal (one alternative, mirroring the kr/ps condition layering).
using ClassifierLiteralVariant = urd::flat_variant<urd::index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>>;

}  // namespace runir::kr::uns

namespace urd
{

// A literal: a boolean feature together with a polarity (true = the feature must hold, false = negated).
template<>
struct data<runir::kr::uns::ClassifierLiteral>
{
    urd::index<runir::kr::uns::ClassifierLiteral> index;
    runir::kr::uns::ClassifierLiteralVariant feature;
    bool polarity = true;

    data() = default;

    data(runir::kr::uns::ClassifierLiteralVariant feature_, bool polarity_) : index(), feature(std::move(feature_)), polarity(polarity_) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"feature", &data::feature>, urd::field<"polarity", &data::polarity>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::feature, &data::polarity> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::feature, &data::polarity> {}; }
};

// A clause: a conjunction of literals.
template<>
struct data<runir::kr::uns::ClassifierClause>
{
    urd::index<runir::kr::uns::ClassifierClause> index;
    urd::flat_vector<urd::index<runir::kr::uns::ClassifierLiteral>> literals;

    data() = default;

    data(urd::flat_vector<urd::index<runir::kr::uns::ClassifierLiteral>> literals_) : index(), literals(std::move(literals_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"literals", &data::literals>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::literals> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::literals> {}; }
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

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"symbol", &data::symbol>, urd::field<"features", &data::features>, urd::field<"clauses", &data::clauses>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::symbol, &data::features, &data::clauses> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::symbol, &data::features, &data::clauses> {}; }
};

}  // namespace urd

#endif
