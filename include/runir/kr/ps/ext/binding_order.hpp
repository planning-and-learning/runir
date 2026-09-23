#ifndef RUNIR_KR_PS_EXT_BINDING_ORDER_HPP_
#define RUNIR_KR_PS_EXT_BINDING_ORDER_HPP_

#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>
#include <tyr/formalism/planning/action_view.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/portable_shuffle.hpp>
#include <yggdrasil/database/relation.hpp>

namespace runir::kr::ps::ext
{

struct InOrder
{
    static constexpr bool shuffled = false;

    void reset(std::uint64_t) const noexcept {}

    template<typename Emit, typename Stop>
    bool for_each_rule(ModuleView module_, Emit&& emit, Stop&& stop) const
    {
        for (const auto transition : module_.get_memory_transitions())
            for (const auto rule : transition)
            {
                if (stop())
                    return false;
                if (!emit(rule))
                    return false;
            }
        return true;
    }

    template<tyr::TaskKind Kind, typename Emit, typename Stop>
    bool for_each_binding(tyr::planning::SuccessorGenerator<Kind>& generator, const tyr::planning::Node<Kind>& node, Emit&& emit, Stop&& stop) const
    {
        const auto visit = [&](tyr::formalism::planning::ActionBindingView binding)
        {
            if (stop())
                return false;
            return emit(binding);
        };
        return generator.for_each_applicable_action_binding(node, std::ref(visit));
    }

    template<tyr::TaskKind Kind, typename Emit, typename Stop>
    bool for_each_binding(tyr::planning::SuccessorGenerator<Kind>& generator,
                          const tyr::planning::Node<Kind>& node,
                          tyr::formalism::planning::ActionView<tyr::LiftedTag> action,
                          Emit&& emit,
                          Stop&& stop) const
    {
        const auto visit = [&](tyr::formalism::planning::ActionBindingView binding)
        {
            if (stop())
                return false;
            return emit(binding);
        };
        return generator.for_each_applicable_action_binding(node, action, std::ref(visit));
    }

    template<runir::kr::dl::CategoryTag Category, typename Emit, typename Stop>
    bool for_each_value(runir::kr::dl::semantics::DenotationView<Category> denotation, Emit&& emit, Stop&& stop) const
    {
        for (const auto value : denotation)
        {
            if (stop())
                return false;
            if (!emit(value))
                return false;
        }
        return true;
    }

    template<typename Emit, typename Stop>
    bool for_each_row(ygg::database::RelationView<> relation, Emit&& emit, Stop&& stop) const
    {
        for (std::size_t i = 0; i < relation.size(); ++i)
        {
            if (stop())
                return false;
            if (!emit(relation[i]))
                return false;
        }
        return true;
    }

    template<typename Iterator>
    void shuffle(Iterator, Iterator) const noexcept
    {
    }
};

class Shuffled
{
private:
    using ConceptCursor = decltype(std::declval<runir::kr::dl::semantics::DenotationView<runir::kr::dl::ConceptTag>>().begin());
    using RoleCursor = decltype(std::declval<runir::kr::dl::semantics::DenotationView<runir::kr::dl::RoleTag>>().begin());

    std::mt19937_64& m_random;
    std::vector<RuleVariantView> m_rules;
    std::vector<tyr::formalism::planning::ActionBindingView> m_bindings;
    std::vector<std::size_t> m_rows;
    std::vector<ConceptCursor> m_concepts;
    std::vector<RoleCursor> m_roles;

    template<typename Enumerate, typename Emit, typename Stop>
    bool visit_bindings(Enumerate&& enumerate, Emit&& emit, Stop&& stop)
    {
        m_bindings.clear();
        const auto collect = [&](tyr::formalism::planning::ActionBindingView binding)
        {
            if (stop())
                return false;
            m_bindings.push_back(binding);
            return true;
        };
        if (!enumerate(std::ref(collect)) || stop())
            return false;
        shuffle(m_bindings.begin(), m_bindings.end());
        for (const auto binding : m_bindings)
        {
            if (stop())
                return false;
            if (!emit(binding))
                return false;
        }
        return true;
    }

public:
    static constexpr bool shuffled = true;

    explicit Shuffled(std::mt19937_64& random) : m_random(random) {}

    void reset(std::uint64_t seed) { m_random.seed(seed); }

    template<typename Emit, typename Stop>
    bool for_each_rule(ModuleView module_, Emit&& emit, Stop&& stop)
    {
        m_rules.clear();
        for (const auto transition : module_.get_memory_transitions())
            for (const auto rule : transition)
            {
                if (stop())
                    return false;
                m_rules.push_back(rule);
            }
        if (stop())
            return false;
        shuffle(m_rules.begin(), m_rules.end());
        for (const auto rule : m_rules)
        {
            if (stop())
                return false;
            if (!emit(rule))
                return false;
        }
        return true;
    }

    template<tyr::TaskKind Kind, typename Emit, typename Stop>
    bool for_each_binding(tyr::planning::SuccessorGenerator<Kind>& generator, const tyr::planning::Node<Kind>& node, Emit&& emit, Stop&& stop)
    {
        return visit_bindings([&](auto visit) { return generator.for_each_applicable_action_binding(node, visit); }, emit, stop);
    }

    template<tyr::TaskKind Kind, typename Emit, typename Stop>
    bool for_each_binding(tyr::planning::SuccessorGenerator<Kind>& generator,
                          const tyr::planning::Node<Kind>& node,
                          tyr::formalism::planning::ActionView<tyr::LiftedTag> action,
                          Emit&& emit,
                          Stop&& stop)
    {
        return visit_bindings([&](auto visit) { return generator.for_each_applicable_action_binding(node, action, visit); }, emit, stop);
    }

    template<runir::kr::dl::CategoryTag Category, typename Emit, typename Stop>
    bool for_each_value(runir::kr::dl::semantics::DenotationView<Category> denotation, Emit&& emit, Stop&& stop)
    {
        auto& cursors = [&]() -> auto&
        {
            if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                return m_concepts;
            else
                return m_roles;
        }();
        cursors.clear();
        for (auto cursor = denotation.begin(); cursor != denotation.end(); ++cursor)
        {
            if (stop())
                return false;
            cursors.push_back(cursor);
        }
        if (stop())
            return false;
        shuffle(cursors.begin(), cursors.end());
        for (const auto cursor : cursors)
        {
            if (stop())
                return false;
            if (!emit(*cursor))
                return false;
        }
        return true;
    }

    template<typename Emit, typename Stop>
    bool for_each_row(ygg::database::RelationView<> relation, Emit&& emit, Stop&& stop)
    {
        m_rows.clear();
        for (std::size_t i = 0; i < relation.size(); ++i)
        {
            if (stop())
                return false;
            m_rows.push_back(i);
        }
        if (stop())
            return false;
        shuffle(m_rows.begin(), m_rows.end());
        for (const auto row : m_rows)
        {
            if (stop())
                return false;
            if (!emit(relation[row]))
                return false;
        }
        return true;
    }

    template<typename Iterator>
    void shuffle(Iterator first, Iterator last)
    {
        ygg::portable_shuffle(first, last, m_random);
    }
};

}  // namespace runir::kr::ps::ext

#endif
