#ifndef RUNIR_KR_UNS_CLASSIFIER_VIEW_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_VIEW_HPP_

#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/uns/classifier_data.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::uns::ClassifierLiteral> C>
class View<Index<runir::kr::uns::ClassifierLiteral>, C> : public formalism::detail::View<Index<runir::kr::uns::ClassifierLiteral>, C>
{
public:
    View(Index<runir::kr::uns::ClassifierLiteral> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::uns::ClassifierLiteral>, C>(handle, context)
    {
    }

    auto get_feature() const noexcept { return make_view(this->get_data().variant, *this->m_context); }
    auto get_polarity() const noexcept { return this->get_data().polarity; }
};

template<formalism::SymbolContextFor<runir::kr::uns::ClassifierClause> C>
class View<Index<runir::kr::uns::ClassifierClause>, C> : public formalism::detail::View<Index<runir::kr::uns::ClassifierClause>, C>
{
public:
    View(Index<runir::kr::uns::ClassifierClause> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::uns::ClassifierClause>, C>(handle, context)
    {
    }

    auto get_literals() const noexcept { return make_view(this->get_data().literals, *this->m_context); }
};

template<formalism::SymbolContextFor<runir::kr::uns::Classifier> C>
class View<Index<runir::kr::uns::Classifier>, C> : public formalism::detail::View<Index<runir::kr::uns::Classifier>, C>
{
public:
    View(Index<runir::kr::uns::Classifier> handle, const C& context) noexcept : formalism::detail::View<Index<runir::kr::uns::Classifier>, C>(handle, context)
    {
    }

    const auto& get_symbol() const noexcept { return this->get_data().symbol; }
    auto get_features() const noexcept { return make_view(this->get_data().features, *this->m_context); }
    auto get_clauses() const noexcept { return make_view(this->get_data().clauses, *this->m_context); }
};

}  // namespace ygg

#endif
