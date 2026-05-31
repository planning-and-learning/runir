#ifndef RUNIR_KR_PS_EXT_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/dl/evaluation_context.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <tyr/formalism/object_index.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
class EvaluationContext
{
public:
    using DlContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>;

    template<runir::kr::dl::CategoryTag Category>
    using Argument = typename DlContext::template Argument<Category>;

    template<runir::kr::dl::CategoryTag Category>
    using Arguments = typename DlContext::template Arguments<Category>;

    using ConceptRegisterValue = std::optional<ygg::Index<tyr::formalism::Object>>;
    using RoleRegisterValue = std::optional<std::pair<ygg::Index<tyr::formalism::Object>, ygg::Index<tyr::formalism::Object>>>;
    using ConceptRegisters = std::array<ConceptRegisterValue, runir::kr::dl::num_registers>;
    using RoleRegisters = std::array<RoleRegisterValue, runir::kr::dl::num_registers>;

    struct CallFrame
    {
        ModuleView module;
        MemoryStateView return_memory_state;
        ConceptRegisters concept_registers;
        RoleRegisters role_registers;
        Arguments<runir::kr::dl::ConceptTag> concept_arguments;
        Arguments<runir::kr::dl::RoleTag> role_arguments;
        Arguments<runir::kr::dl::BooleanTag> boolean_arguments;
        Arguments<runir::kr::dl::NumericalTag> numerical_arguments;
    };

private:
    tyr::planning::StateView<Kind> m_state;
    ModuleView m_module;
    MemoryStateView m_memory_state;
    ConceptRegisters m_concept_registers;
    RoleRegisters m_role_registers;
    Arguments<runir::kr::dl::ConceptTag> m_concept_arguments;
    Arguments<runir::kr::dl::RoleTag> m_role_arguments;
    Arguments<runir::kr::dl::BooleanTag> m_boolean_arguments;
    Arguments<runir::kr::dl::NumericalTag> m_numerical_arguments;
    runir::kr::dl::semantics::Builder* m_dl_builder;
    runir::kr::dl::semantics::DenotationRepository* m_dl_denotation_repository;
    RepositoryPtr m_repository_owner;
    std::vector<ModuleView> m_modules;
    std::vector<CallFrame> m_call_stack;

public:
    EvaluationContext(tyr::planning::StateView<Kind> state,
                      ModuleView module,
                      runir::kr::dl::semantics::Builder& dl_builder,
                      runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository,
                      Arguments<runir::kr::dl::ConceptTag> concept_arguments = {},
                      Arguments<runir::kr::dl::RoleTag> role_arguments = {},
                      Arguments<runir::kr::dl::BooleanTag> boolean_arguments = {},
                      Arguments<runir::kr::dl::NumericalTag> numerical_arguments = {},
                      std::vector<ModuleView> modules = {},
                      RepositoryPtr repository_owner = {}) noexcept :
        m_state(std::move(state)),
        m_module(module),
        m_memory_state(module.get_entry_memory_state()),
        m_concept_registers(),
        m_role_registers(),
        m_concept_arguments(std::move(concept_arguments)),
        m_role_arguments(std::move(role_arguments)),
        m_boolean_arguments(std::move(boolean_arguments)),
        m_numerical_arguments(std::move(numerical_arguments)),
        m_dl_builder(&dl_builder),
        m_dl_denotation_repository(&dl_denotation_repository),
        m_repository_owner(std::move(repository_owner)),
        m_modules(std::move(modules)),
        m_call_stack()
    {
        if (!m_repository_owner)
            m_repository_owner = std::const_pointer_cast<Repository>(m_module.get_context().get_shared_ptr());

        auto contains_entry = false;
        for (auto module : m_modules)
            contains_entry |= module.get_index() == m_module.get_index();
        if (!contains_entry)
            m_modules.push_back(m_module);
    }

    const auto& get_state() const noexcept { return m_state; }
    auto get_module() const noexcept { return m_module; }
    auto get_memory_state() const noexcept { return m_memory_state; }
    const auto& get_modules() const noexcept { return m_modules; }
    const auto& get_repository_owner() const noexcept { return m_repository_owner; }
    const auto& get_call_stack() const noexcept { return m_call_stack; }
    bool has_caller() const noexcept { return !m_call_stack.empty(); }
    auto& get_repository() const noexcept { return m_module.get_context(); }
    auto& get_dl_repository() const noexcept { return m_module.get_context().get_dl_repository(); }
    auto& get_dl_builder() const noexcept { return *m_dl_builder; }
    auto& get_dl_denotation_repository() const noexcept { return *m_dl_denotation_repository; }

    void set_state(tyr::planning::StateView<Kind> state) noexcept { m_state = std::move(state); }
    void set_memory_state(MemoryStateView memory_state) noexcept { m_memory_state = memory_state; }
    void set_memory_state(ygg::Index<MemoryState> memory_state) noexcept { m_memory_state = MemoryStateView(memory_state, get_repository()); }

    std::optional<ModuleView> find_module(const std::string& name) const
    {
        for (auto module : m_modules)
            if (module.get_name() == name)
                return module;
        return std::nullopt;
    }

    void enter_module(ModuleView module,
                      MemoryStateView return_memory_state,
                      Arguments<runir::kr::dl::ConceptTag> concept_arguments = {},
                      Arguments<runir::kr::dl::RoleTag> role_arguments = {},
                      Arguments<runir::kr::dl::BooleanTag> boolean_arguments = {},
                      Arguments<runir::kr::dl::NumericalTag> numerical_arguments = {})
    {
        m_call_stack.push_back(CallFrame { m_module,
                                           return_memory_state,
                                           m_concept_registers,
                                           m_role_registers,
                                           std::move(m_concept_arguments),
                                           std::move(m_role_arguments),
                                           std::move(m_boolean_arguments),
                                           std::move(m_numerical_arguments) });

        m_module = module;
        m_memory_state = module.get_entry_memory_state();
        m_concept_registers = ConceptRegisters {};
        m_role_registers = RoleRegisters {};
        m_concept_arguments = std::move(concept_arguments);
        m_role_arguments = std::move(role_arguments);
        m_boolean_arguments = std::move(boolean_arguments);
        m_numerical_arguments = std::move(numerical_arguments);
    }

    bool restore_caller()
    {
        if (m_call_stack.empty())
            return false;

        auto frame = std::move(m_call_stack.back());
        m_call_stack.pop_back();

        m_module = frame.module;
        m_memory_state = frame.return_memory_state;
        m_concept_registers = std::move(frame.concept_registers);
        m_role_registers = std::move(frame.role_registers);
        m_concept_arguments = std::move(frame.concept_arguments);
        m_role_arguments = std::move(frame.role_arguments);
        m_boolean_arguments = std::move(frame.boolean_arguments);
        m_numerical_arguments = std::move(frame.numerical_arguments);
        return true;
    }

    template<runir::kr::dl::CategoryTag Category>
    const auto& arguments() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return m_concept_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return m_role_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return m_boolean_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return m_numerical_arguments;
    }

    template<runir::kr::dl::CategoryTag Category>
    auto& arguments() noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return m_concept_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return m_role_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return m_boolean_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return m_numerical_arguments;
    }

    auto& concept_registers() noexcept { return m_concept_registers; }
    const auto& concept_registers() const noexcept { return m_concept_registers; }
    auto& role_registers() noexcept { return m_role_registers; }
    const auto& role_registers() const noexcept { return m_role_registers; }

    void set(runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> reg, ygg::Index<tyr::formalism::Object> object)
    {
        concept_registers()[verify_bounds(reg)] = object;
    }

    void
    set(runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag> reg, ygg::Index<tyr::formalism::Object> source, ygg::Index<tyr::formalism::Object> target)
    {
        role_registers()[verify_bounds(reg)] = std::pair(source, target);
    }

    void clear(runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> reg) { concept_registers()[verify_bounds(reg)].reset(); }
    void clear(runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag> reg) { role_registers()[verify_bounds(reg)].reset(); }

    template<typename F>
    decltype(auto) with_dl_context(tyr::planning::StateView<Kind> state, F&& f) const
    {
        auto context = DlContext(std::move(state),
                                 get_dl_builder(),
                                 get_dl_denotation_repository(),
                                 m_concept_arguments,
                                 m_role_arguments,
                                 m_boolean_arguments,
                                 m_numerical_arguments);
        initialize_registers(context);
        return std::forward<F>(f)(context);
    }

    template<typename F>
    decltype(auto) with_current_dl_context(F&& f) const
    {
        return with_dl_context(m_state, std::forward<F>(f));
    }

    template<typename F>
    decltype(auto) with_dl_transition_context(tyr::planning::StateView<Kind> target_state, F&& f) const
    {
        auto context = runir::kr::ps::dl::EvaluationContext<runir::kr::ExtFamilyTag, Kind>(m_state,
                                                                                           std::move(target_state),
                                                                                           get_dl_builder(),
                                                                                           get_dl_denotation_repository(),
                                                                                           m_concept_arguments,
                                                                                           m_role_arguments,
                                                                                           m_boolean_arguments,
                                                                                           m_numerical_arguments);
        initialize_registers(context.get_source_context());
        initialize_registers(context.get_target_context());
        return std::forward<F>(f)(context);
    }

private:
    size_t verify_bounds(runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> reg) const
    {
        const auto index = static_cast<size_t>(ygg::uint_t(reg));
        if (index >= m_concept_registers.size())
            throw std::out_of_range(make_out_of_range_message<runir::kr::dl::ConceptTag>(index));
        return index;
    }

    size_t verify_bounds(runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag> reg) const
    {
        const auto index = static_cast<size_t>(ygg::uint_t(reg));
        if (index >= m_role_registers.size())
            throw std::out_of_range(make_out_of_range_message<runir::kr::dl::RoleTag>(index));
        return index;
    }

    void initialize_registers(DlContext& context) const
    {
        for (size_t index = 0; index < m_concept_registers.size(); ++index)
            if (const auto object = m_concept_registers[index])
                context.set(runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>(static_cast<ygg::uint_t>(index)), ygg::make_view(*object, context));

        for (size_t index = 0; index < m_role_registers.size(); ++index)
            if (const auto role = m_role_registers[index])
                context.set(runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag>(static_cast<ygg::uint_t>(index)),
                            std::pair(ygg::make_view(role->first, context), ygg::make_view(role->second, context)));
    }

    template<runir::kr::dl::CategoryTag Category>
    static std::string make_out_of_range_message(size_t index)
    {
        return std::string(Category::name) + " register identifier " + std::to_string(index) + " is out of range; max registers is "
               + std::to_string(runir::kr::dl::num_registers) + ".";
    }
};

}  // namespace runir::kr::ps::ext

#endif
