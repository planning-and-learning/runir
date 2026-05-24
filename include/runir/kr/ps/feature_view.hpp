#ifndef RUNIR_KR_PS_FEATURE_VIEW_HPP_
#define RUNIR_KR_PS_FEATURE_VIEW_HPP_

#include "runir/kr/ps/feature_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>

namespace tyr
{

template<runir::kr::FamilyTag Family, typename FeatureTag, typename C>
class View<Index<runir::kr::ps::Feature<Family, FeatureTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::Feature<Family, FeatureTag>> m_handle;

public:
    View(Index<runir::kr::ps::Feature<Family, FeatureTag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    template<typename EvaluationContext>
    auto evaluate(EvaluationContext& context) const
    {
        return tyr::visit([&](auto feature) { return feature.evaluate(context); }, get_variant());
    }

    template<tyr::planning::TaskKind Kind, typename EvaluationContext>
    auto evaluate(tyr::planning::StateView<Kind> state, EvaluationContext& context) const
    {
        return tyr::visit([&](auto feature) { return feature.evaluate(state, context); }, get_variant());
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
