#ifndef RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_HPP_

#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/ext/execution_canonicalization.hpp"
#include "runir/kr/ps/ext/module_program_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/task_context.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
class ExecutionRepository
{
private:
    using SymbolRepository = ygg::formalism::SymbolRepository<RegisterValues, CallArguments, CallStack, ExecutionState<Kind>>;

    SymbolRepository m_symbol_repository;
    ExecutionBuilder<Kind> m_builder;
    runir::kr::TaskContextPtr<Kind> m_task_context;
    std::shared_ptr<const Repository> m_program_repository;
    ygg::Index<ModuleProgram> m_program;

    ExecutionRepository(runir::kr::TaskContextPtr<Kind> task_context, std::shared_ptr<const Repository> program_repository, ygg::Index<ModuleProgram> program) :
        m_symbol_repository(nullptr),
        m_builder(),
        m_task_context(std::move(task_context)),
        m_program_repository(std::move(program_repository)),
        m_program(program)
    {
        if (!m_task_context)
            throw std::invalid_argument("ExecutionRepository requires a task context.");
        if (!m_program_repository)
            throw std::invalid_argument("ExecutionRepository requires an owned module-program repository.");
    }

public:
    ExecutionRepository(const ExecutionRepository&) = delete;
    ExecutionRepository& operator=(const ExecutionRepository&) = delete;
    ExecutionRepository(ExecutionRepository&&) = delete;
    ExecutionRepository& operator=(ExecutionRepository&&) = delete;

    static ExecutionRepositoryPtr<Kind> create(runir::kr::TaskContextPtr<Kind> task_context, ModuleProgramView program)
    {
        return ExecutionRepositoryPtr<Kind>(new ExecutionRepository(std::move(task_context), program.get_context().get_shared_ptr(), program.get_index()));
    }

    const auto& get_task_context_ptr() const noexcept { return m_task_context; }
    auto& get_task_context() noexcept { return *m_task_context; }
    const auto& get_task_context() const noexcept { return *m_task_context; }
    const auto& get_program_repository_ptr() const noexcept { return m_program_repository; }
    auto get_program() const noexcept { return ModuleProgramView(m_program, *m_program_repository); }

    auto get_state(ygg::Index<tyr::planning::State<Kind>> state) const { return m_task_context->search_context->state_repository->get_registered_state(state); }

    auto get_object(ygg::Index<tyr::formalism::Object> object) const noexcept
    {
        return ygg::make_view(object, *m_task_context->search_context->task->get_repository());
    }

    template<runir::kr::dl::CategoryTag Category>
    auto get_denotation(ygg::Index<runir::kr::dl::semantics::Denotation<Category>> denotation) const noexcept
    {
        return ygg::make_view(denotation, *m_task_context->dl_denotation_repository);
    }

    auto get_module(ygg::Index<Module> module) const noexcept { return ModuleView(module, *m_program_repository); }
    auto get_memory_state(ygg::Index<MemoryState> memory_state) const noexcept { return MemoryStateView(memory_state, *m_program_repository); }
    auto get_rule(ygg::Index<RuleVariant> rule) const noexcept { return RuleVariantView(rule, *m_program_repository); }

    void clear() noexcept { m_symbol_repository.clear(); }

    template<typename T>
    [[nodiscard]] auto get_builder()
    {
        return m_builder.template get_builder<T>();
    }

    template<typename T>
    std::optional<ygg::Index<T>> find(const ygg::Data<T>& data) const noexcept
    {
        assert(is_canonical(data));
        return m_symbol_repository.template find_local<T>(data);
    }

    template<typename T>
    std::pair<ygg::Index<T>, bool> get_or_create(ygg::Data<T>& data)
    {
        canonicalize(data);
        assert(is_canonical(data));
        return m_symbol_repository.template get_or_create_local<T>(data);
    }

    template<typename T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const noexcept
    {
        return m_symbol_repository.template at_local<T>(index);
    }

    template<typename T>
    size_t size() const noexcept
    {
        return m_symbol_repository.template local_size<T>();
    }
};

}  // namespace runir::kr::ps::ext

#endif
