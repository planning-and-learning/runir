#ifndef RUNIR_KR_UNS_CLASSIFIER_VIEW_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_VIEW_HPP_

#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/uns/classifier_data.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::uns::ClassifierLiteral> C>
class View<Index<runir::kr::uns::ClassifierLiteral>, C> : public ygg::IndexViewBase<runir::kr::uns::ClassifierLiteral, C>
{
public:
    using ygg::IndexViewBase<runir::kr::uns::ClassifierLiteral, C>::IndexViewBase;

    auto get_feature() const noexcept { return make_view(this->get_data().variant, this->get_context()); }
    auto get_polarity() const noexcept { return this->get_data().polarity; }
};

template<formalism::SymbolContextFor<runir::kr::uns::ClassifierClause> C>
class View<Index<runir::kr::uns::ClassifierClause>, C> : public ygg::IndexViewBase<runir::kr::uns::ClassifierClause, C>
{
public:
    using ygg::IndexViewBase<runir::kr::uns::ClassifierClause, C>::IndexViewBase;

    auto get_literals() const noexcept { return make_view(this->get_data().literals, this->get_context()); }
};

template<formalism::SymbolContextFor<runir::kr::uns::Classifier> C>
class View<Index<runir::kr::uns::Classifier>, C> : public ygg::IndexViewBase<runir::kr::uns::Classifier, C>
{
public:
    using ygg::IndexViewBase<runir::kr::uns::Classifier, C>::IndexViewBase;

    const auto& get_symbol() const noexcept { return this->get_data().symbol; }
    auto get_features() const noexcept { return make_view(this->get_data().features, this->get_context()); }
    auto get_clauses() const noexcept { return make_view(this->get_data().clauses, this->get_context()); }
};

}  // namespace ygg

#endif
