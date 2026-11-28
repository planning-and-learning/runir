#ifndef RUNIR_KR_PS_ICP_MODULE_VIEW_HPP_
#define RUNIR_KR_PS_ICP_MODULE_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/icp/module_data.hpp"
#include "runir/kr/ps/icp/reset_pair_view.hpp"
#include "runir/kr/ps/memory_state_view.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <concepts>
#include <cstddef>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace runir::kr::ps::icp
{
/// The syntactic complexity of a module (defined in syntactic_complexity.hpp), declared before its view, whose
/// functions() list it.
template<typename C>
std::size_t syntactic_complexity(const urd::index_view<Module, C>& view);
}  // namespace runir::kr::ps::icp

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::icp::Module, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] auto symbol() const noexcept { return self().template member<&urd::record_of<V>::symbol>(); }

    [[nodiscard]] auto concept_registers() const noexcept { return self().template member<&urd::record_of<V>::concept_registers>(); }

    [[nodiscard]] auto role_registers() const noexcept { return self().template member<&urd::record_of<V>::role_registers>(); }

    [[nodiscard]] auto concept_features() const noexcept { return self().template member<&urd::record_of<V>::concept_features>(); }

    [[nodiscard]] auto role_features() const noexcept { return self().template member<&urd::record_of<V>::role_features>(); }

    [[nodiscard]] auto boolean_features() const noexcept { return self().template member<&urd::record_of<V>::boolean_features>(); }

    [[nodiscard]] auto numerical_features() const noexcept { return self().template member<&urd::record_of<V>::numerical_features>(); }

    [[nodiscard]] auto query_features() const noexcept { return self().template member<&urd::record_of<V>::query_features>(); }

    [[nodiscard]] auto entry_memory_state() const noexcept { return self().template member<&urd::record_of<V>::entry_memory_state>(); }

    [[nodiscard]] auto memory_states() const noexcept { return self().template member<&urd::record_of<V>::memory_states>(); }

    [[nodiscard]] auto memory_transitions() const noexcept { return self().template member<&urd::record_of<V>::memory_transitions>(); }

    [[nodiscard]] auto reset_pairs() const noexcept { return self().template member<&urd::record_of<V>::reset_pairs>(); }

    // user: one getter templated on the tag over the fields it chooses (next to the record), besides the plain getters.
    template<runir::kr::dl::CategoryTag Category>
    [[nodiscard]] auto registers() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_registers();
        else
            return role_registers();
    }

    // user: one getter templated on the tag over the fields it chooses (next to the record), besides the plain getters.
    template<typename FeatureTag>
    [[nodiscard]] auto features() const noexcept
    {
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            return concept_features();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
            return role_features();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag>)
            return boolean_features();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag>)
            return numerical_features();
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::QueryFeature>)
            return query_features();
        else
        {
            static_assert(false, "unhandled feature tag in Module::features");
        }
    }

    static constexpr auto functions() noexcept
    { return urd::Functions<urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::ps::icp::syntactic_complexity)>> {}; }
};
}  // namespace urd

#endif
