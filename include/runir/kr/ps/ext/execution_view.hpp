#ifndef RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_

#include "runir/kr/ps/ext/execution_repository.hpp"

#include <array>
#include <cassert>
#include <concepts>
#include <optional>
#include <tuple>
#include <tyr/formalism/object_view.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<tyr::planning::TaskKind Kind>
class View<Index<runir::kr::ps::ext::RegisterValues>, runir::kr::ps::ext::ExecutionRepositoryPtr<Kind>>
{
private:
    using Context = runir::kr::ps::ext::ExecutionRepositoryPtr<Kind>;

    Index<runir::kr::ps::ext::RegisterValues> m_handle;
    Context m_context;

public:
    using ConceptValue = std::optional<tyr::formalism::planning::ObjectView>;
    using RoleValue = std::optional<std::pair<tyr::formalism::planning::ObjectView, tyr::formalism::planning::ObjectView>>;

    View(Index<runir::kr::ps::ext::RegisterValues> handle, const Context& context) noexcept : m_handle(handle), m_context(context) { assert(m_context); }

    const auto& get_data() const noexcept { return (*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    auto get_concept_values() const
    {
        auto result = std::array<ConceptValue, runir::kr::dl::num_registers> {};
        for (size_t i = 0; i < result.size(); ++i)
            if (const auto& value = get_data().concept_values[i])
                result[i] = m_context->get_object(*value);
        return result;
    }

    auto get_role_values() const
    {
        auto result = std::array<RoleValue, runir::kr::dl::num_registers> {};
        for (size_t i = 0; i < result.size(); ++i)
            if (const auto& value = get_data().role_values[i])
                result[i] = std::pair(m_context->get_object(value->first), m_context->get_object(value->second));
        return result;
    }

    auto identifying_members() const noexcept { return std::make_tuple(m_handle, m_context.get()); }
};

template<tyr::planning::TaskKind Kind>
class View<Index<runir::kr::ps::ext::CallArguments>, runir::kr::ps::ext::ExecutionRepositoryPtr<Kind>>
{
private:
    using Context = runir::kr::ps::ext::ExecutionRepositoryPtr<Kind>;

    Index<runir::kr::ps::ext::CallArguments> m_handle;
    Context m_context;

public:
    View(Index<runir::kr::ps::ext::CallArguments> handle, const Context& context) noexcept : m_handle(handle), m_context(context) { assert(m_context); }

    const auto& get_data() const noexcept { return (*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    template<runir::kr::dl::CategoryTag Category>
    auto get() const
    {
        auto result = std::vector<runir::kr::dl::semantics::DenotationView<Category>> {};
        const auto& data = get_data();
        const auto& indices = [&]() -> const auto&
        {
            if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                return data.concept_arguments;
            else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
                return data.role_arguments;
            else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
                return data.boolean_arguments;
            else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
                return data.numerical_arguments;
            else
                static_assert(ygg::dependent_false<Category>::value, "unhandled execution argument category");
        }();
        result.reserve(indices.size());
        for (auto index : indices)
            result.push_back(m_context->get_denotation(index));
        return result;
    }

    auto identifying_members() const noexcept { return std::make_tuple(m_handle, m_context.get()); }
};

template<tyr::planning::TaskKind Kind>
class View<Index<runir::kr::ps::ext::CallStack>, runir::kr::ps::ext::ExecutionRepositoryPtr<Kind>>
{
private:
    using Context = runir::kr::ps::ext::ExecutionRepositoryPtr<Kind>;

    Index<runir::kr::ps::ext::CallStack> m_handle;
    Context m_context;

public:
    View(Index<runir::kr::ps::ext::CallStack> handle, const Context& context) noexcept : m_handle(handle), m_context(context) { assert(m_context); }

    const auto& get_data() const noexcept { return (*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    auto get_module() const noexcept { return m_context->get_module(get_data().module); }
    auto get_memory_state() const noexcept { return m_context->get_memory_state(get_data().memory_state); }
    auto get_registers() const noexcept { return make_view(get_data().registers, m_context); }
    auto get_arguments() const noexcept { return make_view(get_data().arguments, m_context); }

    auto get_caller() const -> std::optional<View>
    {
        if (!get_data().caller)
            return std::nullopt;
        return View(*get_data().caller, m_context);
    }

    auto identifying_members() const noexcept { return std::make_tuple(m_handle, m_context.get()); }
};

template<tyr::planning::TaskKind Kind>
class View<Index<runir::kr::ps::ext::ExecutionState<Kind>>, runir::kr::ps::ext::ExecutionRepositoryPtr<Kind>>
{
private:
    using Context = runir::kr::ps::ext::ExecutionRepositoryPtr<Kind>;

    Index<runir::kr::ps::ext::ExecutionState<Kind>> m_handle;
    Context m_context;

public:
    View(Index<runir::kr::ps::ext::ExecutionState<Kind>> handle, const Context& context) noexcept : m_handle(handle), m_context(context) { assert(m_context); }

    const auto& get_data() const noexcept { return (*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    auto get_state() const { return m_context->get_state(get_data().state); }
    auto get_program() const noexcept { return m_context->get_program(); }
    auto get_phase() const noexcept { return get_data().phase; }
    auto get_call_stack() const noexcept { return make_view(get_data().call_stack, m_context); }

    auto identifying_members() const noexcept { return std::make_tuple(m_handle, m_context.get()); }
};

}  // namespace ygg

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
using RegisterValuesView = ygg::View<ygg::Index<RegisterValues>, ExecutionRepositoryPtr<Kind>>;

template<tyr::planning::TaskKind Kind>
using CallArgumentsView = ygg::View<ygg::Index<CallArguments>, ExecutionRepositoryPtr<Kind>>;

template<tyr::planning::TaskKind Kind>
using CallStackView = ygg::View<ygg::Index<CallStack>, ExecutionRepositoryPtr<Kind>>;

template<tyr::planning::TaskKind Kind>
using ExecutionStateView = ygg::View<ygg::Index<ExecutionState<Kind>>, ExecutionRepositoryPtr<Kind>>;

}  // namespace runir::kr::ps::ext

#endif
