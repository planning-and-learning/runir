#ifndef RUNIR_KR_UNS_CLASSIFIER_VIEW_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_VIEW_HPP_

#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/uns/classifier_data.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::uns::ClassifierLiteral, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_feature() const noexcept { return self().template member<&urd::record_of<V>::variant>(); }
    auto get_polarity() const noexcept { return self().data().polarity; }
};

template<typename V>
struct view_mixin<runir::kr::uns::ClassifierClause, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_literals() const noexcept { return self().template member<&urd::record_of<V>::literals>(); }
};

template<typename V>
struct view_mixin<runir::kr::uns::Classifier, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_symbol() const noexcept { return self().data().symbol; }
    auto get_features() const noexcept { return self().template member<&urd::record_of<V>::features>(); }
    auto get_clauses() const noexcept { return self().template member<&urd::record_of<V>::clauses>(); }
};
}  // namespace urd

#endif
