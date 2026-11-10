#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_CLOSURE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_CLOSURE_HPP_

#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/dense_set.hpp>
#include <urd/repr/values/sparse_set.hpp>
#include <urd/store.hpp>

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include <urd/bits.hpp>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

/// The rows marked for recomputation, in the order marked.
using MarkedRows = urd::Store<urd::set<urd::index<tyr::formalism::Object>>, urd::dense, urd::insertion_ordered>;

struct ClosureWorkspace
{
    struct Frame
    {
        ygg::uint_t vertex;
        urd::set_bit_indices_view<ygg::uint_t>::Iterator next;
    };
    urd::Store<urd::sequence<size_t>> discovery;
    urd::Store<urd::sequence<size_t>> lowlink;
    urd::Store<urd::set<ygg::uint_t>, urd::dense> active;  ///< The vertices on active_stack.
    urd::Store<urd::sequence<Frame>> frames;
    urd::Store<urd::sequence<ygg::uint_t>> active_stack;
    urd::Store<urd::sequence<ygg::uint_t>> component;

    void initialize(ygg::uint_t num_objects)
    {
        discovery.assign(num_objects, 0);
        lowlink.assign(num_objects, 0);
        active.clear();
        active.reserve(num_objects);
        frames.clear();
        frames.reserve(num_objects);
        active_stack.clear();
        active_stack.reserve(num_objects);
        component.clear();
        component.reserve(num_objects);
    }
};

// Iterative Tarjan SCCs (doi:10.1137/0201010), with component-wise closure reuse
// in the style of Purdom (doi:10.1007/BF01940892). Clean rows must already be final;
// dirty successor SCCs finish first. emit must immediately write each row to result
// so later components can reuse it.
template<BaseRoleConstructorTag Tag, std::invocable<urd::index<tyr::formalism::Object>, urd::bitset_span<const ygg::uint_t>> Emit>
void evaluate_closure(Tag,
                      const urd::builder<Denotation<RoleTag>>& result,
                      const urd::builder<Denotation<RoleTag>>& role,
                      const MarkedRows& dirty,
                      urd::builder<Denotation<ConceptTag>>& row,
                      ClosureWorkspace& scratch,
                      Emit emit)
{
    static_assert(std::same_as<Tag, TransitiveClosureTag> || std::same_as<Tag, ReflexiveTransitiveClosureTag>);
    assert(scratch.discovery.size() == role.get_num_objects());
    const auto is_dirty = [&](ygg::uint_t vertex) { return dirty.contains(urd::index<tyr::formalism::Object>(vertex)); };
    for (const auto source : dirty)
    {
        const auto vertex = static_cast<ygg::uint_t>(source.value);
        scratch.discovery[vertex] = scratch.lowlink[vertex] = 0;
        scratch.active.erase(vertex);
    }
    scratch.frames.clear();
    scratch.active_stack.clear();
    scratch.component.clear();
    size_t index = 0;
    const auto discover = [&](ygg::uint_t vertex)
    {
        scratch.discovery[vertex] = scratch.lowlink[vertex] = ++index;
        scratch.active.insert(vertex);
        scratch.active_stack.push_back(vertex);
        scratch.frames.push_back({ vertex, urd::set_bit_indices(role.get(urd::index<tyr::formalism::Object>(vertex))).begin() });
    };
    for (const auto source : dirty)
    {
        if (scratch.discovery[static_cast<ygg::uint_t>(source.value)])
            continue;
        discover(static_cast<ygg::uint_t>(source.value));
        while (!scratch.frames.empty())
        {
            auto& frame = scratch.frames.back();
            const auto vertex = frame.vertex;
            if (frame.next != std::default_sentinel)
            {
                const auto target = static_cast<ygg::uint_t>(*frame.next);
                ++frame.next;
                if (is_dirty(target))
                {
                    if (!scratch.discovery[target])
                        discover(target);
                    else if (scratch.active.contains(target))
                        scratch.lowlink[vertex] = std::min(scratch.lowlink[vertex], scratch.discovery[target]);
                }
                continue;
            }
            scratch.frames.pop_back();
            if (!scratch.frames.empty())
            {
                const auto parent = scratch.frames.back().vertex;
                scratch.lowlink[parent] = std::min(scratch.lowlink[parent], scratch.lowlink[vertex]);
            }
            if (scratch.lowlink[vertex] != scratch.discovery[vertex])
                continue;
            scratch.component.clear();
            while (true)
            {
                const auto member = scratch.active_stack.back();
                scratch.active_stack.pop_back();
                scratch.active.erase(member);
                scratch.component.push_back(member);
                if (member == vertex)
                    break;
            }

            auto reached = row.get();
            reached.reset();
            if (std::same_as<Tag, ReflexiveTransitiveClosureTag> || scratch.component.size() > 1)
                for (const auto member : scratch.component)
                    reached.set(member);
            for (const auto member : scratch.component)
                for (const auto neighbor : urd::set_bit_indices(role.get(urd::index<tyr::formalism::Object>(member))))
                {
                    const auto target = static_cast<ygg::uint_t>(neighbor);
                    if (!reached.test(target))
                    {
                        reached.set(target);
                        // A strict singleton self-edge must not reuse its unfinished row.
                        if (target != member)
                        {
                            assert(!is_dirty(target) || (scratch.discovery[target] && !scratch.active.contains(target)));
                            reached |= result.get(urd::index<tyr::formalism::Object>(target));
                        }
                    }
                }
            for (const auto member : scratch.component)
                emit(urd::index<tyr::formalism::Object>(member), urd::bitset_span<const ygg::uint_t>(reached));
        }
    }
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
