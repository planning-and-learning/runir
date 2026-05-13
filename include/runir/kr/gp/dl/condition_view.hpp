#ifndef RUNIR_KR_GP_DL_CONDITION_VIEW_HPP_
#define RUNIR_KR_GP_DL_CONDITION_VIEW_HPP_

#include "runir/kr/gp/dl/condition_data.hpp"
#include "runir/kr/gp/dl/evaluation_context.hpp"
#include "runir/kr/gp/dl/feature_view.hpp"
#include "runir/kr/gp/feature_view.hpp"

#include <concepts>
#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename FeatureTag, typename ObservationTag, typename C>
class View<Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>> m_handle;

public:
    View(Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>> handle, const C& context) noexcept :
        m_context(&context),
        m_handle(handle)
    {
    }

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_feature() const noexcept { return make_view(get_data().feature, *m_context); }

    template<tyr::planning::TaskKind Kind>
    bool is_compatible_with(runir::kr::gp::dl::EvaluationContext<Kind>& context) const
    {
        const auto value = get_feature().evaluate(context.get_source_context());

        if constexpr (std::same_as<FeatureTag, runir::kr::gp::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::gp::dl::Positive>)
            return value;
        else if constexpr (std::same_as<FeatureTag, runir::kr::gp::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::gp::dl::Negative>)
            return !value;
        else if constexpr (std::same_as<FeatureTag, runir::kr::gp::dl::NumericalFeature> && std::same_as<ObservationTag, runir::kr::gp::dl::EqualZero>)
            return value == 0;
        else if constexpr (std::same_as<FeatureTag, runir::kr::gp::dl::NumericalFeature> && std::same_as<ObservationTag, runir::kr::gp::dl::GreaterZero>)
            return value > 0;
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
