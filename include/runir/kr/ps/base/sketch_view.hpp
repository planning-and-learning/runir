#ifndef RUNIR_KR_PS_BASE_SKETCH_VIEW_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_VIEW_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_data.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::base::Sketch> C>
class View<Index<runir::kr::ps::base::Sketch>, C> : public formalism::detail::View<Index<runir::kr::ps::base::Sketch>, C>
{
public:
    View(Index<runir::kr::ps::base::Sketch> handle, const C& context) noexcept : formalism::detail::View<Index<runir::kr::ps::base::Sketch>, C>(handle, context)
    {
    }

    template<typename FeatureTag>
    auto get_features() const noexcept
    {
        if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
            return make_view(this->get_data().boolean_features, *this->m_context);
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
            return make_view(this->get_data().numerical_features, *this->m_context);
        else
        {
            static_assert(ygg::dependent_false<FeatureTag>::value, "unhandled feature tag in Sketch::get_features");
        }
    }

    auto get_rules() const noexcept { return make_view(this->get_data().rules, *this->m_context); }
};

}  // namespace ygg

#endif
