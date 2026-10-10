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
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::icp::Module> C>
class View<Index<runir::kr::ps::icp::Module>, C> : public ygg::IndexViewBase<runir::kr::ps::icp::Module, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::icp::Module, C>::IndexViewBase;

    auto get_symbol() const noexcept { return make_view(this->get_data().symbol, this->get_context()); }
    const auto& get_name() const noexcept { return get_symbol().get_name(); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_registers() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(this->get_data().concept_registers, this->get_repository().get_dl_repository());
        else
            return make_view(this->get_data().role_registers, this->get_repository().get_dl_repository());
    }

    template<typename FeatureTag>
    auto get_features() const noexcept
    {
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            return make_view(this->get_data().concept_features, this->get_context());
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
            return make_view(this->get_data().role_features, this->get_context());
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag>)
            return make_view(this->get_data().boolean_features, this->get_context());
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag>)
            return make_view(this->get_data().numerical_features, this->get_context());
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::QueryFeature>)
            return make_view(this->get_data().query_features, this->get_context());
        else
        {
            static_assert(ygg::dependent_false<FeatureTag>::value, "unhandled feature tag in Module::get_features");
        }
    }

    auto get_query_features() const noexcept { return get_features<runir::kr::ps::dl::QueryFeature>(); }

    auto get_entry_memory_state() const noexcept { return make_view(this->get_data().entry_memory_state, this->get_context()); }
    auto get_memory_states() const noexcept { return make_view(this->get_data().memory_states, this->get_context()); }
    auto get_memory_transitions() const noexcept { return make_view(this->get_data().memory_transitions, this->get_context()); }

    const auto& get_reset_pairs() const noexcept { return this->get_data().reset_pairs; }
    auto get_concept_features() const noexcept { return get_features<runir::kr::dl::ConceptTag>(); }
};

}  // namespace ygg

#endif
