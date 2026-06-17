#ifndef RUNIR_KR_UNS_CLASSIFIER_DATA_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_DATA_HPP_

#include "runir/kr/uns/classifier_index.hpp"
#include "runir/kr/uns/feature_index.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

// A literal: a boolean feature (wrapped in a single-alternative variant, mirroring the kr/ps condition
// layering) together with a polarity (true = the feature must hold, false = negated).
template<>
struct Data<runir::kr::uns::ClassifierLiteral>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::uns::Feature>>;

    Index<runir::kr::uns::ClassifierLiteral> index;
    Variant value;
    bool polarity = true;

    Data() = default;
    Data(Variant value_, bool polarity_) : index(), value(std::move(value_)), polarity(polarity_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(value);
        ygg::clear(polarity);
    }

    auto cista_members() const noexcept { return std::tie(index, value, polarity); }
    auto identifying_members() const noexcept { return std::tie(value, polarity); }
};

// A clause: a conjunction of literals.
template<>
struct Data<runir::kr::uns::ClassifierClause>
{
    Index<runir::kr::uns::ClassifierClause> index;
    IndexList<runir::kr::uns::ClassifierLiteral> literals;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(literals);
    }

    auto cista_members() const noexcept { return std::tie(index, literals); }
    auto identifying_members() const noexcept { return std::tie(literals); }
};

// A classifier: named boolean features + a DNF (disjunction of clauses).
template<>
struct Data<runir::kr::uns::Classifier>
{
    Index<runir::kr::uns::Classifier> index;
    ::cista::offset::string symbol;
    ::cista::offset::string description;
    IndexList<runir::kr::uns::Feature> features;
    IndexList<runir::kr::uns::ClassifierClause> clauses;

    Data() = default;
    Data(::cista::offset::string symbol_, ::cista::offset::string description_) : index(), symbol(std::move(symbol_)), description(std::move(description_)) {}
    Data(const std::string& symbol_, const std::string& description_) : index(), symbol(symbol_), description(description_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(symbol);
        ygg::clear(description);
        ygg::clear(features);
        ygg::clear(clauses);
    }

    auto cista_members() const noexcept { return std::tie(index, symbol, description, features, clauses); }
    auto identifying_members() const noexcept { return std::tie(symbol, description, features, clauses); }
};

}  // namespace ygg

#endif
