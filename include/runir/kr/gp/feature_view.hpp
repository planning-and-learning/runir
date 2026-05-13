#ifndef RUNIR_KR_GP_FEATURE_VIEW_HPP_
#define RUNIR_KR_GP_FEATURE_VIEW_HPP_

#include "runir/kr/gp/feature_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>

namespace tyr
{

template<typename FeatureTag, typename C>
class View<Index<runir::kr::gp::Feature<FeatureTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::gp::Feature<FeatureTag>> m_handle;

public:
    View(Index<runir::kr::gp::Feature<FeatureTag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    template<tyr::planning::TaskKind Kind, typename EvaluationContext>
    auto evaluate(tyr::planning::StateView<Kind> state, EvaluationContext& context) const
    {
        return get_variant().apply([&](auto feature) { return feature.evaluate(state, context); });
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
