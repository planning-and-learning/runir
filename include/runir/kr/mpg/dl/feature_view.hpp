#ifndef RUNIR_KR_MPG_DL_FEATURE_VIEW_HPP_
#define RUNIR_KR_MPG_DL_FEATURE_VIEW_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/mpg/dl/evaluation_context.hpp"
#include "runir/kr/mpg/dl/feature_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename FeatureTag, typename C>
class View<Index<runir::kr::mpg::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::mpg::ConcreteFeature<runir::kr::DlTag, FeatureTag>> m_handle;

public:
    View(Index<runir::kr::mpg::ConcreteFeature<runir::kr::DlTag, FeatureTag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_feature() const noexcept
    {
        if constexpr (std::same_as<FeatureTag, runir::kr::mpg::dl::ConceptFeature>)
            return make_view(get_data().feature, *m_context);
        else
            return make_view(get_data().feature, m_context->get_dl_repository());
    }
    const auto& get_symbol() const noexcept { return get_data().symbol; }
    const auto& get_description() const noexcept { return get_data().description; }

    template<tyr::planning::TaskKind Kind>
    auto evaluate(runir::kr::dl::semantics::EvaluationContext<Kind>& context) const
    {
        return runir::kr::dl::semantics::evaluate(get_feature(), context).get();
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
