#ifndef RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/memory_state_view.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <concepts>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::ext::Module, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] auto get_symbol() const noexcept { return self().template member<&urd::record_of<V>::symbol>(); }
    [[nodiscard]] auto get_concept_arguments() const noexcept { return self().template member<&urd::record_of<V>::concept_arguments>(); }
    [[nodiscard]] auto get_role_arguments() const noexcept { return self().template member<&urd::record_of<V>::role_arguments>(); }
    [[nodiscard]] auto get_boolean_arguments() const noexcept { return self().template member<&urd::record_of<V>::boolean_arguments>(); }
    [[nodiscard]] auto get_numerical_arguments() const noexcept { return self().template member<&urd::record_of<V>::numerical_arguments>(); }
    [[nodiscard]] auto get_concept_registers() const noexcept { return self().template member<&urd::record_of<V>::concept_registers>(); }
    [[nodiscard]] auto get_role_registers() const noexcept { return self().template member<&urd::record_of<V>::role_registers>(); }
    [[nodiscard]] auto get_concept_features() const noexcept { return self().template member<&urd::record_of<V>::concept_features>(); }
    [[nodiscard]] auto get_role_features() const noexcept { return self().template member<&urd::record_of<V>::role_features>(); }
    [[nodiscard]] auto get_boolean_features() const noexcept { return self().template member<&urd::record_of<V>::boolean_features>(); }
    [[nodiscard]] auto get_numerical_features() const noexcept { return self().template member<&urd::record_of<V>::numerical_features>(); }
    [[nodiscard]] auto get_query_features() const noexcept { return self().template member<&urd::record_of<V>::query_features>(); }
    [[nodiscard]] auto get_entry_memory_state() const noexcept { return self().template member<&urd::record_of<V>::entry_memory_state>(); }
    [[nodiscard]] auto get_memory_states() const noexcept { return self().template member<&urd::record_of<V>::memory_states>(); }
    [[nodiscard]] auto get_memory_transitions() const noexcept { return self().template member<&urd::record_of<V>::memory_transitions>(); }

    // user: one getter templated on the tag over the fields it chooses (next to the record), besides the plain getters.
    template<runir::kr::dl::CategoryTag Category>
    [[nodiscard]] auto get_arguments() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return get_concept_arguments();
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return get_role_arguments();
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return get_boolean_arguments();
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return get_numerical_arguments();
    }
    // user: one getter templated on the tag over the fields it chooses (next to the record), besides the plain getters.
    template<runir::kr::dl::CategoryTag Category>
    [[nodiscard]] auto get_registers() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return get_concept_registers();
        else
            return get_role_registers();
    }

    // user: one getter templated on the tag over the fields it chooses (next to the record), besides the plain getters.
    template<typename FeatureTag>
    [[nodiscard]] auto get_features() const noexcept
    {
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            return get_concept_features();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
            return get_role_features();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag>)
            return get_boolean_features();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag>)
            return get_numerical_features();
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::QueryFeature>)
            return get_query_features();
        else
        {
            static_assert(false, "unhandled feature tag in Module::get_features");
        }
    }
};
}  // namespace urd

#endif
