#ifndef RUNIR_KR_PS_EXT_DETAIL_TRANSIENT_STATE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_TRANSIENT_STATE_HPP_

#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/pooled_shared_owner.hpp"
#include "runir/kr/ps/ext/detail/transient_values.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include <yggdrasil/containers/unique_object_pool.hpp>
#include <yggdrasil/semantics/containers/dynamic_bitset_equal_to.hpp>
#include <yggdrasil/semantics/containers/dynamic_bitset_hash.hpp>
#include <yggdrasil/semantics/equal_to.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
using StateBuilderPool = ygg::UniqueObjectPool<ygg::Builder<tyr::planning::State<Kind>>>;

template<tyr::TaskKind Kind>
struct TransientPlanningState;
template<tyr::TaskKind Kind>
struct TransientModuleData;
struct TransientCallStack;

template<tyr::TaskKind Kind>
using TransientPlanningPtr = PooledSharedOwner<TransientPlanningState<Kind>>;
template<tyr::TaskKind Kind>
using TransientModulePtr = PooledSharedOwner<TransientModuleData<Kind>>;
using TransientCallerPtr = PooledSharedOwner<TransientCallStack>;
using TransientArgumentsPtr = PooledSharedOwner<OwnedCallArguments>;

/// Immutable after construction. Shared control transitions retain one pooled planning payload.
template<tyr::TaskKind Kind>
struct TransientPlanningState
{
    // The pointer must return its builder before the pool can be destroyed.
    std::shared_ptr<StateBuilderPool<Kind>> pool;
    ygg::UniqueObjectPoolPtr<ygg::Builder<tyr::planning::State<Kind>>> builder;

    TransientPlanningState() = default;
    TransientPlanningState(const TransientPlanningState&) = delete;
    TransientPlanningState& operator=(const TransientPlanningState&) = delete;
    TransientPlanningState(TransientPlanningState&&) = delete;
    TransientPlanningState& operator=(TransientPlanningState&&) = delete;

    void initialize(std::shared_ptr<StateBuilderPool<Kind>> pool_)
    {
        pool = std::move(pool_);
        builder = pool->get_or_allocate();
    }
    void release_owners() noexcept
    {
        builder = {};
        pool.reset();
    }
    auto identifying_members() const noexcept { return std::tie(builder->template get_atoms<tyr::formalism::FluentTag>(), builder->get_numeric_variables()); }
};

template<tyr::TaskKind Kind>
struct TransientModuleData
{
    TransientPlanningPtr<Kind> state;
    ygg::Index<Module> module_;
    ygg::Index<MemoryState> memory_state;
    OwnedRegisterValues registers;
    TransientArgumentsPtr arguments;
    void release_owners() noexcept
    {
        state.reset();
        arguments.reset();
    }
    auto identifying_members() const noexcept { return std::tie(*state, module_, memory_state, registers, *arguments); }
};

struct TransientCallStack
{
    ygg::Index<Module> module_;
    ygg::Index<MemoryState> return_memory_state;
    OwnedRegisterValues registers;
    TransientArgumentsPtr arguments;
    TransientCallerPtr caller;

    void release_owners() noexcept
    {
        arguments.reset();
        auto tail = std::move(caller);
        while (tail && tail.ref_count() == 1)
        {
            auto next = std::move(tail->caller);
            tail.reset();
            tail = std::move(next);
        }
    }

    const auto& get_data() const noexcept { return *this; }
    const auto& get_caller() const noexcept { return caller; }
    auto identifying_members() const noexcept
    {
        return std::tuple_cat(std::tie(module_, return_memory_state, registers, *arguments),
                              std::make_tuple(caller ? std::optional(std::cref(*caller)) : std::nullopt));
    }
};

template<tyr::TaskKind Kind>
class TransientModuleView
{
    const TransientModuleData<Kind>* m_data;
    const runir::kr::TaskContext<Kind>* m_context;

public:
    TransientModuleView(const TransientModuleData<Kind>& data, const runir::kr::TaskContext<Kind>& context) : m_data(&data), m_context(&context) {}
    const auto& get_data() const noexcept { return *m_data; }
    auto get_state() const { return tyr::planning::BuilderStateView<Kind>(*m_data->state->builder, *m_context->search_context->task); }
    auto get_module() const { return ygg::make_view(m_data->module_, *m_context->domain_context->ext_repository); }
    auto get_memory_state() const { return ygg::make_view(m_data->memory_state, *m_context->domain_context->ext_repository); }
    auto get_registers() const { return RegisterValuesRef(m_data->registers, *m_context->search_context->task->get_repository()); }
    auto get_arguments() const { return CallArgumentsRef(*m_data->arguments); }
};

template<tyr::TaskKind Kind>
class TransientProgramState
{
    runir::kr::TaskContextPtr<Kind> m_context;
    ProgramView m_program;
    TransientModulePtr<Kind> m_module;
    TransientCallerPtr m_caller;

public:
    TransientProgramState(runir::kr::TaskContextPtr<Kind> context, ProgramView program, TransientModulePtr<Kind> module_, TransientCallerPtr caller) :
        m_context(std::move(context)),
        m_program(program),
        m_module(std::move(module_)),
        m_caller(std::move(caller))
    {
    }

    const auto& get_context() const noexcept { return *m_context->execution_repository; }
    auto get_program() const noexcept { return m_program; }
    auto get_module_state() const { return TransientModuleView<Kind>(*m_module, *m_context); }
    auto get_state() const { return get_module_state().get_state(); }
    const auto& get_call_stack() const noexcept { return m_caller; }
    auto identifying_members() const noexcept
    {
        return std::tuple_cat(std::tie(m_program, *m_module), std::make_tuple(m_caller ? std::optional(std::cref(*m_caller)) : std::nullopt));
    }
};

template<runir::kr::dl::CategoryTag Category>
struct TransientChoice
{
    using CategoryType = Category;
    using Binding = typename Choice<Category>::Binding;
    using BindingPool = ygg::UniqueObjectPool<std::vector<Binding>>;

private:
    std::shared_ptr<BindingPool> m_pool;
    ygg::UniqueObjectPoolPtr<std::vector<Binding>> m_bindings;

public:
    RuleVariantView rule;
    size_t position = 0;

    TransientChoice(RuleVariantView rule_, runir::kr::dl::semantics::DenotationView<Category> denotation, std::shared_ptr<BindingPool> pool) :
        m_pool(std::move(pool)),
        m_bindings(m_pool->get_or_allocate()),
        rule(rule_)
    {
        m_bindings->clear();
        for (const auto binding : denotation)
            m_bindings->push_back(binding);
    }
    TransientChoice(TransientChoice&&) noexcept = default;
    TransientChoice& operator=(TransientChoice&& other) noexcept
    {
        if (this != &other)
        {
            m_bindings = {};
            m_pool = std::move(other.m_pool);
            m_bindings = std::move(other.m_bindings);
            rule = other.rule;
            position = other.position;
        }
        return *this;
    }
    auto& bindings() noexcept { return *m_bindings; }
    const auto& bindings() const noexcept { return *m_bindings; }
    bool exhausted() const noexcept { return position == bindings().size(); }
    const auto& current() const { return bindings().at(position); }
    void advance() noexcept { ++position; }
    bool has_alternatives() const noexcept { return bindings().size() > 1; }
    size_t count() const noexcept { return bindings().size(); }
};

template<tyr::TaskKind Kind>
struct TransientLabeledNode
{
    TransientPlanningPtr<Kind> owner;
    tyr::formalism::planning::ActionBindingView label;
    tyr::planning::Node<tyr::planning::BuilderStateView<Kind>> node;
};

template<tyr::TaskKind Kind>
class TransientPools
{
    std::shared_ptr<StateBuilderPool<Kind>> m_builders = std::make_shared<StateBuilderPool<Kind>>();
    std::shared_ptr<ygg::SharedObjectPool<TransientPlanningState<Kind>>> m_planning = std::make_shared<ygg::SharedObjectPool<TransientPlanningState<Kind>>>();
    std::shared_ptr<ygg::SharedObjectPool<TransientModuleData<Kind>>> m_modules = std::make_shared<ygg::SharedObjectPool<TransientModuleData<Kind>>>();
    std::shared_ptr<ygg::SharedObjectPool<TransientCallStack>> m_callers = std::make_shared<ygg::SharedObjectPool<TransientCallStack>>();
    std::shared_ptr<ygg::SharedObjectPool<OwnedCallArguments>> m_arguments = std::make_shared<ygg::SharedObjectPool<OwnedCallArguments>>();
    std::shared_ptr<typename TransientChoice<transient_dl::ConceptTag>::BindingPool> m_concept_bindings =
        std::make_shared<typename TransientChoice<transient_dl::ConceptTag>::BindingPool>();
    std::shared_ptr<typename TransientChoice<transient_dl::RoleTag>::BindingPool> m_role_bindings =
        std::make_shared<typename TransientChoice<transient_dl::RoleTag>::BindingPool>();

public:
    auto planning()
    {
        auto result = TransientPlanningPtr<Kind>(m_planning);
        result->initialize(m_builders);
        return result;
    }
    auto module_() { return TransientModulePtr<Kind>(m_modules); }
    auto caller() { return TransientCallerPtr(m_callers); }
    auto arguments()
    {
        auto result = TransientArgumentsPtr(m_arguments);
        result->initialize();
        return result;
    }
    template<transient_dl::ConceptOrRoleTag Category>
    auto choice(RuleVariantView rule, transient_semantics::DenotationView<Category> denotation)
    {
        if constexpr (std::same_as<Category, transient_dl::ConceptTag>)
            return TransientChoice<Category>(rule, denotation, m_concept_bindings);
        else
            return TransientChoice<Category>(rule, denotation, m_role_bindings);
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
