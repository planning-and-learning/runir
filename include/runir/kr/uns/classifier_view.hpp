#ifndef RUNIR_KR_UNS_CLASSIFIER_VIEW_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_VIEW_HPP_

#include "runir/kr/uns/classifier_data.hpp"
#include "runir/kr/uns/feature_view.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::uns::ClassifierLiteral>, C>
{
private:
    const C* m_context;
    Index<runir::kr::uns::ClassifierLiteral> m_handle;

public:
    View(Index<runir::kr::uns::ClassifierLiteral> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_feature() const noexcept { return make_view(get_data().value, *m_context); }
    auto get_polarity() const noexcept { return get_data().polarity; }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

template<typename C>
class View<Index<runir::kr::uns::ClassifierClause>, C>
{
private:
    const C* m_context;
    Index<runir::kr::uns::ClassifierClause> m_handle;

public:
    View(Index<runir::kr::uns::ClassifierClause> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_literals() const noexcept { return make_view(get_data().literals, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

template<typename C>
class View<Index<runir::kr::uns::Classifier>, C>
{
private:
    const C* m_context;
    Index<runir::kr::uns::Classifier> m_handle;

public:
    View(Index<runir::kr::uns::Classifier> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    const auto& get_symbol() const noexcept { return get_data().symbol; }
    const auto& get_description() const noexcept { return get_data().description; }
    auto get_features() const noexcept { return make_view(get_data().features, *m_context); }
    auto get_clauses() const noexcept { return make_view(get_data().clauses, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
