#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_BUILDER_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_BUILDER_HPP_

#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"

#include <cstddef>
#include <optional>
#include <ranges>
#include <tyr/formalism/object_builder.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of RegisterValues: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::semantics::RegisterValues, C>
{
    urd::index<runir::kr::dl::semantics::RegisterValues> index;
    urd::built_t<urd::flat_vector<urd::flat_optional<urd::index<tyr::formalism::Object>>>, ::tyr::formalism::planning::Repository> concept_values;
    urd::built_t<urd::flat_vector<urd::flat_optional<urd::flat_pair<urd::index<tyr::formalism::Object>, urd::index<tyr::formalism::Object>>>>,
                 ::tyr::formalism::planning::Repository>
        role_values;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"concept_values", &builder::concept_values>, urd::field<"role_values", &builder::role_values>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::concept_values, &builder::role_values> {}; }
};

}  // namespace urd

namespace runir::kr::dl::semantics
{

/// Replace a register builder's values with source's, each object as its interned view, retaining buffers. Each value
/// is written at its position after it is read, so a view of the builder itself leaves it as it is.
template<RegisterValuesViewConcept R>
urd::builder<RegisterValues, DenotationRepository>& assign(urd::builder<RegisterValues, DenotationRepository>& built, R source)
{
    using Object = urd::child<tyr::formalism::Object, tyr::formalism::planning::Repository>;  // an object: the planning repository's
    const auto concept_values = source.template values<ConceptTag>();
    built.concept_values.resize(std::ranges::size(concept_values));
    auto concept_at = std::size_t { 0 };
    for (const auto& value : concept_values)
        built.concept_values[concept_at++] = value.has_value() ? std::optional<Object>(std::in_place, *value) : std::optional<Object>();
    const auto role_values = source.template values<RoleTag>();
    built.role_values.resize(std::ranges::size(role_values));
    auto role_at = std::size_t { 0 };
    for (const auto& value : role_values)
        built.role_values[role_at++] = value.has_value() ? std::optional<std::pair<Object, Object>>(std::in_place, value->first, value->second) :
                                                           std::optional<std::pair<Object, Object>>();
    built.index = {};
    return built;
}

}  // namespace runir::kr::dl::semantics

#endif
