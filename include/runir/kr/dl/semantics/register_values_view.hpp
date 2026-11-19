#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"

#include <concepts>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/object_view.hpp>
#include <urd/elements/builder.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <variant>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::dl::semantics::detail
{
/// A register's object, interned: one of the task's (a built object has no index in the task: throws std::logic_error).
struct object_of
{
    tyr::formalism::planning::ObjectView operator()(const auto& object) const
    {
        if (const auto* interned = std::get_if<urd::interned_child<tyr::formalism::Object, tyr::formalism::planning::Repository>>(&object))
            return *interned;
        throw std::logic_error("a register's object is one of the task's: interned");
    }
};

/// A concept register's value, its object interned.
struct concept_value_of
{
    std::optional<tyr::formalism::planning::ObjectView> operator()(const auto& value) const
    { return value.has_value() ? std::optional(object_of {}(*value)) : std::nullopt; }
};

/// A role register's value, its objects interned.
struct role_value_of
{
    std::optional<std::pair<tyr::formalism::planning::ObjectView, tyr::formalism::planning::ObjectView>> operator()(const auto& value) const
    { return value.has_value() ? std::optional(std::pair(object_of {}(value->first), object_of {}(value->second))) : std::nullopt; }
};
}  // namespace runir::kr::dl::semantics::detail

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::dl::semantics::RegisterValues, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_concept_values() const noexcept { return self().template member<&urd::record_of<V>::concept_values>(); }

    auto get_role_values() const noexcept { return self().template member<&urd::record_of<V>::role_values>(); }

    /// The formalism repository its objects belong to: its denotation repository's.
    const auto& get_formalism_repository() const noexcept { return self().context().get_formalism_repository(); }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto values() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return get_concept_values();
        else
            return get_role_values();
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto at(runir::kr::dl::RegisterIdentifier<Category> reg) const
    { return values<Category>().at(static_cast<size_t>(ygg::uint_t(reg.value))); }

    static constexpr auto functions() noexcept
    {
        return urd::Functions<urd::function<"get_formalism_repository", &view_mixin::get_formalism_repository>,
                              urd::function<"at", &view_mixin::template at<runir::kr::dl::ConceptTag>, urd::arg<"reg">>,
                              urd::function<"at", &view_mixin::template at<runir::kr::dl::RoleTag>, urd::arg<"reg">>> {};
    }
};

/// A register builder read as its record: each register's object, one of the task's, as its interned view
/// (detail::object_of).
template<typename C, typename V>
struct view_mixin<builder<runir::kr::dl::semantics::RegisterValues, C>, V> : view_mixin<runir::kr::dl::semantics::RegisterValues, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_concept_values() const noexcept
    { return urd::transform(self().template member<&urd::record_of<V>::concept_values>(), runir::kr::dl::semantics::detail::concept_value_of {}); }

    auto get_role_values() const noexcept
    { return urd::transform(self().template member<&urd::record_of<V>::role_values>(), runir::kr::dl::semantics::detail::role_value_of {}); }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto values() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return get_concept_values();
        else
            return get_role_values();
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto at(runir::kr::dl::RegisterIdentifier<Category> reg) const
    { return values<Category>().at(static_cast<size_t>(ygg::uint_t(reg.value))); }

    static constexpr auto functions() noexcept
    {
        return urd::Functions<urd::function<"get_formalism_repository", &view_mixin::get_formalism_repository>,
                              urd::function<"at", &view_mixin::template at<runir::kr::dl::ConceptTag>, urd::arg<"reg">>,
                              urd::function<"at", &view_mixin::template at<runir::kr::dl::RoleTag>, urd::arg<"reg">>> {};
    }
};

}  // namespace urd

#endif
