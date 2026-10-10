#ifndef RUNIR_KR_UNS_CLASSIFIER_DATA_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_DATA_HPP_

#include "runir/kr/ps/feature_index.hpp"
#include "runir/kr/uns/classifier_index.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

// A literal: a boolean feature (wrapped in a single-alternative variant, mirroring the kr/ps condition
// layering) together with a polarity (true = the feature must hold, false = negated).
template<>
struct Data<runir::kr::uns::ClassifierLiteral>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>>>;

    Index<runir::kr::uns::ClassifierLiteral> index;
    Variant variant;
    bool polarity = true;

    Data() = default;
    Data(Variant variant_, bool polarity_) : index(), variant(std::move(variant_)), polarity(polarity_) {}

    auto cista_members() noexcept { return std::tie(index, variant, polarity); }
    auto cista_members() const noexcept { return std::tie(index, variant, polarity); }
    auto identifying_members() const noexcept { return std::tie(variant, polarity); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

// A clause: a conjunction of literals.
template<>
struct Data<runir::kr::uns::ClassifierClause>
{
    Index<runir::kr::uns::ClassifierClause> index;
    IndexList<runir::kr::uns::ClassifierLiteral> literals;

    Data() = default;
    Data(IndexList<runir::kr::uns::ClassifierLiteral> literals_) : index(), literals(std::move(literals_)) {}
    template<typename C>
    Data(const std::vector<::ygg::View<Index<runir::kr::uns::ClassifierLiteral>, C>>& literals_) : index(), literals()
    {
        set(literals_, literals);
    }

    auto cista_members() noexcept { return std::tie(index, literals); }
    auto cista_members() const noexcept { return std::tie(index, literals); }
    auto identifying_members() const noexcept { return std::tie(literals); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

// A classifier: named boolean features + a DNF (disjunction of clauses).
template<>
struct Data<runir::kr::uns::Classifier>
{
    Index<runir::kr::uns::Classifier> index;
    ::cista::offset::string symbol;
    IndexList<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>> features;
    IndexList<runir::kr::uns::ClassifierClause> clauses;

    Data() = default;
    Data(::cista::offset::string symbol_,
         IndexList<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>> features_,
         IndexList<runir::kr::uns::ClassifierClause> clauses_) :
        index(),
        symbol(std::move(symbol_)),
        features(std::move(features_)),
        clauses(std::move(clauses_))
    {
    }
    template<typename C>
    Data(::cista::offset::string symbol_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>>, C>>& features_,
         const std::vector<::ygg::View<Index<runir::kr::uns::ClassifierClause>, C>>& clauses_) :
        index(),
        symbol(std::move(symbol_)),
        features(),
        clauses()
    {
        set(features_, features);
        set(clauses_, clauses);
    }

    auto cista_members() noexcept { return std::tie(index, symbol, features, clauses); }
    auto cista_members() const noexcept { return std::tie(index, symbol, features, clauses); }
    auto identifying_members() const noexcept { return std::tie(symbol, features, clauses); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
