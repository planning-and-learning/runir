#ifndef RUNIR_KR_PS_BASE_SKETCH_VIEW_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_VIEW_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_data.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::base::Sketch> C>
class View<Index<runir::kr::ps::base::Sketch>, C> : public ygg::IndexViewBase<runir::kr::ps::base::Sketch, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::base::Sketch, C>::IndexViewBase;

    template<typename FeatureTag>
    auto get_features() const noexcept
    {
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag>)
            return make_view(this->get_data().boolean_features, this->get_context());
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag>)
            return make_view(this->get_data().numerical_features, this->get_context());
        else
        {
            static_assert(ygg::dependent_false<FeatureTag>::value, "unhandled feature tag in Sketch::get_features");
        }
    }

    auto get_rules() const noexcept { return make_view(this->get_data().rules, this->get_context()); }
};

}  // namespace ygg

#endif
