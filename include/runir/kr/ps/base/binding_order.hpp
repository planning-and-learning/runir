#ifndef RUNIR_KR_PS_BASE_BINDING_ORDER_HPP_
#define RUNIR_KR_PS_BASE_BINDING_ORDER_HPP_

#include <functional>
#include <random>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <vector>
#include <yggdrasil/core/portable_shuffle.hpp>

namespace runir::kr::ps::base
{

struct InOrder
{
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
};

class Shuffled
{
private:
    std::mt19937_64& m_random;
    std::vector<tyr::formalism::planning::ActionBindingView> m_bindings;

public:
    explicit Shuffled(std::mt19937_64& random) : m_random(random) {}

    template<tyr::TaskKind Kind, typename Emit, typename Stop>
    bool for_each_binding(tyr::planning::SuccessorGenerator<Kind>& generator, const tyr::planning::Node<Kind>& node, Emit&& emit, Stop&& stop)
    {
        // Collect only bindings; successor states are generated when emit visits them.
        m_bindings.clear();
        const auto collect = [&](tyr::formalism::planning::ActionBindingView binding)
        {
            if (stop())
                return false;
            m_bindings.push_back(binding);
            return true;
        };
        const auto complete = generator.for_each_applicable_action_binding(node, std::ref(collect));
        if (!complete || stop())
            return false;

        ygg::portable_shuffle(m_bindings.begin(), m_bindings.end(), m_random);
        for (auto binding : m_bindings)
        {
            if (stop())
                return false;
            if (!emit(binding))
                return false;
        }
        return true;
    }
};

}  // namespace runir::kr::ps::base

#endif
