#ifndef RUNIR_KR_UNS_CLASSIFIER_VIEW_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_VIEW_HPP_

#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/uns/classifier_data.hpp"

#include <cstddef>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace runir::kr::uns
{
/// The syntactic complexity of a classifier (defined in syntactic_complexity.hpp), declared before its view, whose
/// functions() list it.
template<typename C>
std::size_t syntactic_complexity(const urd::index_view<Classifier, C>& view);
}  // namespace runir::kr::uns

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::uns::ClassifierLiteral, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto feature() const noexcept { return self().template member<&urd::record_of<V>::feature>(); }

    auto polarity() const noexcept { return self().template member<&urd::record_of<V>::polarity>(); }
};

template<typename V>
struct view_mixin<runir::kr::uns::ClassifierClause, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto literals() const noexcept { return self().template member<&urd::record_of<V>::literals>(); }
};

template<typename V>
struct view_mixin<runir::kr::uns::Classifier, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& symbol() const noexcept { return self().template member<&urd::record_of<V>::symbol>(); }

    auto features() const noexcept { return self().template member<&urd::record_of<V>::features>(); }

    auto clauses() const noexcept { return self().template member<&urd::record_of<V>::clauses>(); }

    static constexpr auto functions() noexcept
    { return urd::Functions<urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::uns::syntactic_complexity)>> {}; }
};
}  // namespace urd

#endif
