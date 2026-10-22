#ifndef RUNIR_KR_DL_SEMANTICS_STATE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_STATE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view_concepts.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

namespace detail
{

/// Defined in the library, so that the contexts need only declarations of their resources.
EvaluationWorkspace& get_workspace(Builder& builder) noexcept;
const tyr::formalism::planning::Repository* get_formalism_repository(const DenotationRepository& repository) noexcept;
const ygg::Data<RegisterValues>& empty_register_values() noexcept;
[[noreturn]] void throw_unbound_register(size_t index, size_t count);
[[noreturn]] void throw_unbound_argument(size_t index, size_t count);
[[noreturn]] void throw_unbound_arguments();

}  // namespace detail

/// Evaluation reads state contents; it does not require registered state identity.
template<typename Context, typename Kind>
concept StateEvaluationResourcesConcept = tyr::TaskKind<Kind> && requires(Context& context, const Context& const_context) {
    { const_context.get_state() } -> tyr::planning::StateViewConcept<Kind>;
    { context.get_builder() } -> std::same_as<Builder&>;
    { context.get_denotation_repository() } -> std::same_as<DenotationRepository&>;
    { context.get_workspace() } -> std::same_as<EvaluationWorkspace&>;
    { context.get_caches() } -> std::same_as<DenotationCaches&>;
    { const_context.registers() } -> std::same_as<BorrowedRegisterValuesView>;
    { const_context.has_arguments() } -> std::same_as<bool>;
    { const_context.arguments() } -> std::same_as<CallArgumentsView>;
};

/// Recursive contexts retain their family, task kind, resources, and navigation.
template<typename Context, typename Kind>
concept StateEvaluationNavigationConcept = StateEvaluationResourcesConcept<Context, Kind> && requires(const Context& context) {
    { context.for_result(false) } -> std::same_as<Context>;
    { context.child_context() } -> std::same_as<Context>;
};

template<typename Context, typename Kind>
concept StateEvaluationContextConcept =
    StateEvaluationResourcesConcept<std::remove_reference_t<Context>, Kind> && requires(const std::remove_reference_t<Context>& context) {
        { context.for_result(false) } -> StateEvaluationNavigationConcept<Kind>;
        { context.child_context() } -> StateEvaluationNavigationConcept<Kind>;
    };

/// Registers in the borrowed form, which interned registers provide without copying.
template<RegisterValuesViewConcept R>
BorrowedRegisterValuesView borrow_registers(R registers) noexcept
{
    return BorrowedRegisterValuesView(registers.get_data(), registers.get_formalism_repository());
}

/// Every context carries registers and call arguments. Registers are empty and arguments absent unless a program binds
/// them; evaluating a register or an argument that the context does not bind throws.
template<tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> S = tyr::planning::StateView<Kind>>
class StateEvaluationContext
{
private:
    S m_state;
    Builder& m_builder;
    DenotationCaches* m_caches;
    std::array<DenotationRepository*, 2> m_denotations;
    EvaluationStorage& m_intermediates;
    BorrowedRegisterValuesView m_registers;
    std::optional<CallArgumentsView> m_arguments;
    bool m_result_is_static = false;

    void select_result(bool is_static) noexcept { m_result_is_static = is_static; }
    void select_children() noexcept
    {
        m_caches = &m_intermediates.get_caches();
        m_denotations = { &m_intermediates.get_denotation_repository(false), &m_intermediates.get_denotation_repository(true) };
    }

    void validate_inputs() const
    {
        const auto* repository = m_state.get_formalism_repository().get();
        if (m_arguments && detail::get_formalism_repository(m_arguments->get_context()) != repository)
            throw std::invalid_argument("Evaluation requires arguments for the same planning task.");
        if (&m_registers.get_formalism_repository() != repository)
            throw std::invalid_argument("Evaluation requires registers for the same planning task.");
    }

public:
    using KindType = Kind;

    StateEvaluationContext(S state, Builder& builder, EvaluationStorage& storage) :
        m_state(std::move(state)),
        m_builder(builder),
        m_caches(&storage.get_caches()),
        m_denotations { &storage.get_denotation_repository(false), &storage.get_denotation_repository(true) },
        m_intermediates(storage),
        m_registers(detail::empty_register_values(), *m_state.get_formalism_repository())
    {
        if (storage.get_denotation_repository(false).get_formalism_repository_ptr() != m_state.get_formalism_repository())
            throw std::invalid_argument("Evaluation requires a result repository for the same planning task.");
    }

    /// Retain computed roots in an existing repository while children use reusable storage.
    StateEvaluationContext(S state, Builder& builder, DenotationCaches& caches, DenotationRepository& repository, EvaluationStorage& intermediates) :
        StateEvaluationContext(std::move(state), builder, intermediates)
    {
        if (detail::get_formalism_repository(repository) != m_state.get_formalism_repository().get())
            throw std::invalid_argument("Evaluation requires a result repository for the same planning task.");
        m_caches = &caches;
        m_denotations = { &repository, &repository };
    }

    template<RegisterValuesViewConcept R>
    StateEvaluationContext(S state, Builder& builder, EvaluationStorage& storage, CallArgumentsView arguments, R registers) :
        StateEvaluationContext(std::move(state), builder, storage)
    {
        m_registers = borrow_registers(registers);
        m_arguments = arguments;
        validate_inputs();
    }

    template<RegisterValuesViewConcept R>
    StateEvaluationContext(S state,
                           Builder& builder,
                           DenotationCaches& caches,
                           DenotationRepository& repository,
                           EvaluationStorage& intermediates,
                           CallArgumentsView arguments,
                           R registers) :
        StateEvaluationContext(std::move(state), builder, caches, repository, intermediates)
    {
        m_registers = borrow_registers(registers);
        m_arguments = arguments;
        validate_inputs();
    }

    const S& get_state() const noexcept { return m_state; }
    Builder& get_builder() noexcept { return m_builder; }
    DenotationRepository& get_denotation_repository(bool is_static) const noexcept { return *m_denotations[is_static]; }
    DenotationRepository& get_denotation_repository() const noexcept { return get_denotation_repository(m_result_is_static); }
    EvaluationWorkspace& get_workspace() noexcept { return detail::get_workspace(m_builder); }
    DenotationCaches& get_caches() noexcept { return *m_caches; }

    BorrowedRegisterValuesView registers() const noexcept { return m_registers; }
    bool has_arguments() const noexcept { return m_arguments.has_value(); }
    CallArgumentsView arguments() const
    {
        if (!m_arguments)
            detail::throw_unbound_arguments();
        return *m_arguments;
    }

    /// Copies only borrowed views and references; result payloads stay put.
    StateEvaluationContext for_result(bool is_static) const noexcept
    {
        auto result = *this;
        result.select_result(is_static);
        return result;
    }

    /// Recursive results use the reusable intermediate caches and repositories.
    StateEvaluationContext child_context() const noexcept
    {
        auto result = *this;
        result.select_children();
        return result;
    }
};

/// The value of a register, which only programs bind.
template<ConceptOrRoleTag Category, typename Context>
auto register_value(const Context& context, RegisterIdentifier<Category> identifier)
{
    const auto values = context.registers().template get<Category>();
    if (ygg::uint_t(identifier) >= values.size())
        detail::throw_unbound_register(ygg::uint_t(identifier), values.size());
    return values[ygg::uint_t(identifier)];
}

/// The value of a call argument, which only ext programs bind.
template<CategoryTag Category, typename Context>
auto argument_value(const Context& context, ArgumentIdentifier<Category> identifier)
{
    const auto values = context.arguments().template get<Category>();
    if (ygg::uint_t(identifier) >= values.size())
        detail::throw_unbound_argument(ygg::uint_t(identifier), values.size());
    return values[ygg::uint_t(identifier)];
}

}

#endif
