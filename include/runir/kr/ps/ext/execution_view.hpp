#ifndef RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_

#include "runir/kr/ps/ext/execution_repository.hpp"

#include <concepts>
#include <optional>
#include <tuple>
#include <tyr/formalism/object_view.hpp>
#include <vector>
#include <yggdrasil/containers/array.hpp>
#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/pair.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::ext::RegisterValues>, C>
{
private:
    Index<runir::kr::ps::ext::RegisterValues> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::ps::ext::RegisterValues> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    auto get_concept_values() const noexcept { return make_view(get_data().concept_values, get_repository(*m_context).get_formalism_repository()); }

    auto get_role_values() const noexcept { return make_view(get_data().role_values, get_repository(*m_context).get_formalism_repository()); }

    auto identifying_members() const noexcept { return std::make_tuple(get_handle(), m_context->get_index()); }
};

template<typename C>
class View<Index<runir::kr::ps::ext::CallArguments>, C>
{
private:
    Index<runir::kr::ps::ext::CallArguments> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::ps::ext::CallArguments> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
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
            result.push_back(make_view(index, get_repository(*m_context).get_denotation_repository()));
        return result;
    }

    auto identifying_members() const noexcept { return std::make_tuple(get_handle(), m_context->get_index()); }
};

template<typename C>
class View<Index<runir::kr::ps::ext::CallStack>, C>
{
private:
    Index<runir::kr::ps::ext::CallStack> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::ps::ext::CallStack> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    auto get_module() const noexcept { return make_view(get_data().module, get_repository(*m_context).get_program_repository()); }
    auto get_memory_state() const noexcept { return make_view(get_data().memory_state, get_repository(*m_context).get_program_repository()); }
    auto get_registers() const noexcept { return make_view(get_data().registers, *m_context); }
    auto get_arguments() const noexcept { return make_view(get_data().arguments, *m_context); }

    auto get_caller() const -> std::optional<View>
    {
        if (!get_data().caller)
            return std::nullopt;
        return make_view(*get_data().caller, *m_context);
    }

    auto identifying_members() const noexcept { return std::make_tuple(get_handle(), m_context->get_index()); }
};

template<tyr::planning::TaskKind Kind, typename C>
class View<Index<runir::kr::ps::ext::ExecutionState<Kind>>, C>
{
private:
    Index<runir::kr::ps::ext::ExecutionState<Kind>> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::ps::ext::ExecutionState<Kind>> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    auto get_state() const { return get_repository(*m_context).get_state_repository().get_registered_state(get_data().state); }
    auto get_program() const noexcept { return make_view(get_data().program, get_repository(*m_context).get_program_repository()); }
    auto get_phase() const noexcept { return get_data().phase; }
    auto get_call_stack() const noexcept { return make_view(get_data().call_stack, *m_context); }

    auto identifying_members() const noexcept { return std::make_tuple(get_handle(), m_context->get_index()); }
};

}  // namespace ygg

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
using RegisterValuesView = ygg::View<ygg::Index<RegisterValues>, ExecutionRepository<Kind>>;

template<tyr::planning::TaskKind Kind>
using CallArgumentsView = ygg::View<ygg::Index<CallArguments>, ExecutionRepository<Kind>>;

template<tyr::planning::TaskKind Kind>
using CallStackView = ygg::View<ygg::Index<CallStack>, ExecutionRepository<Kind>>;

template<tyr::planning::TaskKind Kind>
using ExecutionStateView = ygg::View<ygg::Index<ExecutionState<Kind>>, ExecutionRepository<Kind>>;

}  // namespace runir::kr::ps::ext

#endif
