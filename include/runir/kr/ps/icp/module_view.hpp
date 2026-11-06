#ifndef RUNIR_KR_PS_ICP_MODULE_VIEW_HPP_
#define RUNIR_KR_PS_ICP_MODULE_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/ps/icp/memory_state_view.hpp"
#include "runir/kr/ps/icp/module_data.hpp"
#include "runir/kr/ps/icp/module_symbol_view.hpp"
#include "runir/kr/ps/icp/rule_variant_view.hpp"

#include <concepts>
#include <tuple>
#include <urd/repr/core/dependent_false.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::icp::Module, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_symbol() const noexcept { return self().template get<&urd::record_of<V>::symbol>(); }
    const auto& get_name() const noexcept { return get_symbol().get_name(); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_registers() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return urd::resolve(self().data().concept_registers, get_repository(self().context()).get_dl_repository());
        else
            return urd::resolve(self().data().role_registers, get_repository(self().context()).get_dl_repository());
    }

    template<typename FeatureTag>
    auto get_features() const noexcept
    {
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            return self().template get<&urd::record_of<V>::concept_features>();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
            return self().template get<&urd::record_of<V>::role_features>();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag>)
            return self().template get<&urd::record_of<V>::boolean_features>();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag>)
            return self().template get<&urd::record_of<V>::numerical_features>();
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::QueryFeature>)
            return self().template get<&urd::record_of<V>::query_features>();
        else
        {
            static_assert(urd::repr::dependent_false<FeatureTag>::value, "unhandled feature tag in Module::get_features");
        }
    }

    auto get_query_features() const noexcept { return get_features<runir::kr::ps::dl::QueryFeature>(); }

    auto get_entry_memory_state() const noexcept { return self().template get<&urd::record_of<V>::entry_memory_state>(); }
    auto get_memory_states() const noexcept { return self().template get<&urd::record_of<V>::memory_states>(); }
    auto get_memory_transitions() const noexcept { return self().template get<&urd::record_of<V>::memory_transitions>(); }

    const auto& get_reset_pairs() const noexcept { return self().data().reset_pairs; }
    auto get_concept_features() const noexcept { return get_features<runir::kr::dl::ConceptTag>(); }
};
}  // namespace urd

#endif
