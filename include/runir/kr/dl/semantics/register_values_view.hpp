#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"

#include <concepts>
#include <cstddef>
#include <optional>
#include <ranges>
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
/// A register's object, interned: one of the task's, read through its repository (a built object has no index in the
/// task: throws std::logic_error).
struct object_of
{
    tyr::formalism::ObjectView operator()(const tyr::formalism::Repository* repository, const auto& object) const
    {
        if (const auto* interned = std::get_if<urd::interned_child<tyr::formalism::Object, tyr::formalism::Repository>>(&object))
            return urd::resolve(*interned, *repository);
        throw std::logic_error("a register's object is one of the task's: interned");
    }
};

/// A concept register's value, its object interned.
struct concept_value_of
{
    std::optional<tyr::formalism::ObjectView> operator()(const tyr::formalism::Repository* repository, const auto& value) const
    { return value.has_value() ? std::optional(object_of {}(repository, *value)) : std::nullopt; }
};

/// A role register's value, its objects interned.
struct role_value_of
{
    std::optional<std::pair<tyr::formalism::ObjectView, tyr::formalism::ObjectView>> operator()(const tyr::formalism::Repository* repository,
                                                                                                const auto& value) const
    { return value.has_value() ? std::optional(std::pair(object_of {}(repository, value->first), object_of {}(repository, value->second))) : std::nullopt; }
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
    auto concept_values() const noexcept { return self().template member<&urd::record_of<V>::concept_values>(); }

    auto role_values() const noexcept { return self().template member<&urd::record_of<V>::role_values>(); }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto values() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_values();
        else
            return role_values();
    }

    /// The value of register `reg` (std::out_of_range beyond its registers).
    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto at(runir::kr::dl::RegisterIdentifier<Category> reg) const
    {
        auto registers = values<Category>();
        const auto position = static_cast<std::size_t>(ygg::uint_t(reg.value));
        if (position >= std::ranges::size(registers))
            throw std::out_of_range("RegisterValues::at: no such register");
        return registers[static_cast<std::ptrdiff_t>(position)];
    }

    static constexpr auto functions() noexcept
    {
        return urd::Functions<urd::function<"at", &view_mixin::template at<runir::kr::dl::ConceptTag>, urd::arg<"reg">>,
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
    auto concept_values() const noexcept
    {
        return urd::transform(urd::detail::view_access::record(self()).concept_values,
                              &self().context().template get<tyr::formalism::Repository>(),
                              runir::kr::dl::semantics::detail::concept_value_of {});
    }

    auto role_values() const noexcept
    {
        return urd::transform(urd::detail::view_access::record(self()).role_values,
                              &self().context().template get<tyr::formalism::Repository>(),
                              runir::kr::dl::semantics::detail::role_value_of {});
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto values() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_values();
        else
            return role_values();
    }

    /// The value of register `reg` (std::out_of_range beyond its registers).
    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto at(runir::kr::dl::RegisterIdentifier<Category> reg) const
    {
        auto registers = values<Category>();
        const auto position = static_cast<std::size_t>(ygg::uint_t(reg.value));
        if (position >= std::ranges::size(registers))
            throw std::out_of_range("RegisterValues::at: no such register");
        return registers[static_cast<std::ptrdiff_t>(position)];
    }

    static constexpr auto functions() noexcept
    {
        return urd::Functions<urd::function<"at", &view_mixin::template at<runir::kr::dl::ConceptTag>, urd::arg<"reg">>,
                              urd::function<"at", &view_mixin::template at<runir::kr::dl::RoleTag>, urd::arg<"reg">>> {};
    }
};

}  // namespace urd

#endif
