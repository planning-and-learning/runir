#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_SET_OPERATIONS_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_SET_OPERATIONS_HPP_

#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <yggdrasil/core/concepts.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

/// Scratch shared by sequential constructor updates; no denotation owns it.
struct SetOperationWorkspace
{
    std::vector<tyr::formalism::planning::ObjectView> rows;
    std::vector<std::uint8_t> marked;
    ygg::Builder<Denotation<ConceptTag>> row;
    std::vector<ygg::uint_t> queue;

    void initialize(ygg::uint_t num_objects)
    {
        rows.clear();
        rows.reserve(num_objects);
        marked.assign(num_objects, 0);
        row.initialize(num_objects);
        queue.clear();
        queue.reserve(num_objects);
    }

    void mark(tyr::formalism::planning::ObjectView source)
    {
        const auto index = ygg::uint_t(source.get_index());
        if (!marked[index])
        {
            rows.push_back(source);
            marked[index] = 1;
        }
    }

    void clear_rows() noexcept
    {
        for (const auto source : rows)
            marked[ygg::uint_t(source.get_index())] = 0;
        rows.clear();
    }

    void mark_all(const tyr::formalism::planning::Repository& repository)
    {
        clear_rows();
        for (ygg::uint_t source = 0; source < marked.size(); ++source)
            mark(ygg::make_view(ygg::Index<tyr::formalism::Object>(source), repository));
    }
};

template<ConceptOrRoleTag Category, std::invocable<DenotationElementView<Category>> Function>
void for_changed(const DenotationDelta<Category>& delta, Function&& function)
{
    for (const auto& value : delta.added)
        function(value);
    for (const auto& value : delta.removed)
        function(value);
}

template<ConceptOrRoleTag Category>
bool contains(BorrowedDenotationView<Category> input, DenotationElementView<Category> value)
{
    if constexpr (std::same_as<Category, ConceptTag>)
        return input.get().test(ygg::uint_t(value.get_index()));
    else
        return input.get(value.first.get_index()).test(ygg::uint_t(value.second.get_index()));
}

inline void mark_sources(SetOperationWorkspace& workspace, const DenotationDelta<RoleTag>& delta)
{
    for_changed(delta, [&](auto edge) { workspace.mark(edge.first); });
}

inline void mark_predecessors(SetOperationWorkspace& workspace, BorrowedDenotationView<RoleTag> role, tyr::formalism::planning::ObjectView target)
{
    const auto index = ygg::uint_t(target.get_index());
    for (ygg::uint_t source = 0; source < role.get_num_objects(); ++source)
        if (role.get(source).test(index))
            workspace.mark(ygg::make_view(ygg::Index<tyr::formalism::Object>(source), role.get_formalism_repository()));
}

inline void mark_predecessors(SetOperationWorkspace& workspace, BorrowedDenotationView<RoleTag> role, const DenotationDelta<ConceptTag>& delta)
{
    for_changed(delta, [&](auto target) { mark_predecessors(workspace, role, target); });
}

inline void replace_row(DenotationState<RoleTag>& output, tyr::formalism::planning::ObjectView source, BorrowedDenotationView<ConceptTag> row)
{
    const auto previous = output.get_result(row.get_formalism_repository());
    // Removing the current bit does not disturb traversal of the remaining bits.
    for (const auto target : previous.range(source))
        if (!contains(row, target))
            output.set({ source, target }, false);
    for (const auto target : row)
        output.set({ source, target }, true);
}

template<BaseConceptConstructorTag Tag>
bool number_restriction(size_t count, ygg::uint_t threshold)
{
    if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag> || std::same_as<Tag, QualifiedAtLeastNumberRestrictionTag>)
        return count >= threshold;
    else if constexpr (std::same_as<Tag, AtMostNumberRestrictionTag> || std::same_as<Tag, QualifiedAtMostNumberRestrictionTag>)
        return count <= threshold;
    else
    {
        static_assert(std::same_as<Tag, ExactNumberRestrictionTag> || std::same_as<Tag, QualifiedExactNumberRestrictionTag>);
        return count == threshold;
    }
}

inline void evaluate_rows(RestrictionTag,
                          DenotationState<RoleTag>& output,
                          BorrowedDenotationView<RoleTag> role,
                          BorrowedDenotationView<ConceptTag> concept_,
                          SetOperationWorkspace& workspace)
{
    for (const auto source : workspace.rows)
    {
        auto row = workspace.row.get();
        row.copy_from(role.get(source.get_index()));
        row &= concept_.get();
        replace_row(output, source, ygg::make_view(workspace.row, role.get_formalism_repository()));
    }
}

template<BaseConceptConstructorTag Tag>
void evaluate_rows(Tag,
                   DenotationState<ConceptTag>& output,
                   BorrowedDenotationView<RoleTag> role,
                   BorrowedDenotationView<ConceptTag> concept_,
                   SetOperationWorkspace& workspace)
{
    static_assert(std::same_as<Tag, ValueRestrictionTag> || std::same_as<Tag, ExistentialQuantificationTag>);
    for (const auto source : workspace.rows)
    {
        const auto present = std::same_as<Tag, ValueRestrictionTag> ? role.get(source.get_index()).is_subset_of(concept_.get()) :
                                                                      role.get(source.get_index()).intersects(concept_.get());
        output.set(source, present);
    }
}

template<BaseConceptConstructorTag Tag>
void evaluate_rows(Tag, DenotationState<ConceptTag>& output, BorrowedDenotationView<RoleTag> role, ygg::uint_t threshold, SetOperationWorkspace& workspace)
{
    for (const auto source : workspace.rows)
        output.set(source, number_restriction<Tag>(role.get(source.get_index()).count(), threshold));
}

template<BaseConceptConstructorTag Tag>
void evaluate_rows(Tag,
                   DenotationState<ConceptTag>& output,
                   BorrowedDenotationView<RoleTag> role,
                   BorrowedDenotationView<ConceptTag> concept_,
                   ygg::uint_t threshold,
                   SetOperationWorkspace& workspace)
{
    for (const auto source : workspace.rows)
    {
        const auto count = role.get(source.get_index()).count_intersection(concept_.get());
        output.set(source, number_restriction<Tag>(count, threshold));
    }
}

template<BaseConceptConstructorTag Tag>
void evaluate_rows(Tag,
                   DenotationState<ConceptTag>& output,
                   BorrowedDenotationView<RoleTag> lhs,
                   BorrowedDenotationView<RoleTag> rhs,
                   SetOperationWorkspace& workspace)
{
    static_assert(std::same_as<Tag, RoleValueMapTag> || std::same_as<Tag, AgreementTag>);
    for (const auto source : workspace.rows)
    {
        const auto present = std::same_as<Tag, RoleValueMapTag> ? lhs.get(source.get_index()).is_subset_of(rhs.get(source.get_index())) :
                                                                  lhs.get(source.get_index()) == rhs.get(source.get_index());
        output.set(source, present);
    }
}

template<ygg::SizedForwardRangeOf<tyr::formalism::planning::ObjectView> Objects>
void evaluate_rows(RoleFillersTag,
                   DenotationState<ConceptTag>& output,
                   BorrowedDenotationView<RoleTag> role,
                   const Objects& fillers,
                   SetOperationWorkspace& workspace)
{
    for (const auto source : workspace.rows)
    {
        bool present = true;
        for (const auto filler : fillers)
            if (!role.get(source.get_index()).test(ygg::uint_t(filler.get_index())))
            {
                present = false;
                break;
            }
        output.set(source, present);
    }
}

inline void evaluate_rows(CompositionTag,
                          DenotationState<RoleTag>& output,
                          BorrowedDenotationView<RoleTag> lhs,
                          BorrowedDenotationView<RoleTag> rhs,
                          SetOperationWorkspace& workspace)
{
    for (const auto source : workspace.rows)
    {
        auto row = workspace.row.get();
        row.reset();
        for (const auto middle : ygg::set_bit_indices(lhs.get(source.get_index())))
            row |= rhs.get(static_cast<ygg::uint_t>(middle));
        replace_row(output, source, ygg::make_view(workspace.row, lhs.get_formalism_repository()));
    }
}

template<BaseRoleConstructorTag Tag>
void evaluate_rows(Tag, DenotationState<RoleTag>& output, BorrowedDenotationView<RoleTag> role, SetOperationWorkspace& workspace)
{
    static_assert(std::same_as<Tag, TransitiveClosureTag> || std::same_as<Tag, ReflexiveTransitiveClosureTag>);
    for (const auto source : workspace.rows)
    {
        auto reached = workspace.row.get();
        reached.reset();
        workspace.queue.clear();
        const auto enqueue = [&](ygg::uint_t target)
        {
            if (!reached.test(target))
            {
                reached.set(target);
                workspace.queue.push_back(target);
            }
        };
        if constexpr (std::same_as<Tag, ReflexiveTransitiveClosureTag>)
            enqueue(ygg::uint_t(source.get_index()));
        for (const auto target : ygg::set_bit_indices(role.get(source.get_index())))
            enqueue(static_cast<ygg::uint_t>(target));
        for (size_t position = 0; position < workspace.queue.size(); ++position)
            for (const auto target : ygg::set_bit_indices(role.get(workspace.queue[position])))
                enqueue(static_cast<ygg::uint_t>(target));
        replace_row(output, source, ygg::make_view(workspace.row, role.get_formalism_repository()));
    }
}

template<ConceptOrRoleTag Category>
void initialize_set(IntersectionTag,
                    DenotationState<Category>& output,
                    BorrowedDenotationView<Category> lhs,
                    const DenotationDelta<Category>&,
                    BorrowedDenotationView<Category> rhs,
                    const DenotationDelta<Category>&,
                    SetOperationWorkspace&)
{
    output.initialize(lhs.get_num_objects());
    const auto update = [&](DenotationElementView<Category> value) { output.set(value, contains(lhs, value) && contains(rhs, value)); };
    for (const auto value : lhs)
        update(value);
    output.clear_delta();
}

template<ConceptOrRoleTag Category>
void update_set(IntersectionTag,
                DenotationState<Category>& output,
                BorrowedDenotationView<Category> lhs,
                const DenotationDelta<Category>& lhs_delta,
                BorrowedDenotationView<Category> rhs,
                const DenotationDelta<Category>& rhs_delta,
                SetOperationWorkspace&)
{
    output.clear_delta();
    const auto update = [&](DenotationElementView<Category> value) { output.set(value, contains(lhs, value) && contains(rhs, value)); };
    for_changed(lhs_delta, update);
    for_changed(rhs_delta, update);
}

template<ConceptOrRoleTag Category>
void initialize_set(UnionTag,
                    DenotationState<Category>& output,
                    BorrowedDenotationView<Category> lhs,
                    const DenotationDelta<Category>&,
                    BorrowedDenotationView<Category> rhs,
                    const DenotationDelta<Category>&,
                    SetOperationWorkspace&)
{
    output.initialize(lhs.get_num_objects());
    const auto update = [&](DenotationElementView<Category> value) { output.set(value, contains(lhs, value) || contains(rhs, value)); };
    for (const auto value : lhs)
        update(value);
    for (const auto value : rhs)
        update(value);
    output.clear_delta();
}

template<ConceptOrRoleTag Category>
void update_set(UnionTag,
                DenotationState<Category>& output,
                BorrowedDenotationView<Category> lhs,
                const DenotationDelta<Category>& lhs_delta,
                BorrowedDenotationView<Category> rhs,
                const DenotationDelta<Category>& rhs_delta,
                SetOperationWorkspace&)
{
    output.clear_delta();
    const auto update = [&](DenotationElementView<Category> value) { output.set(value, contains(lhs, value) || contains(rhs, value)); };
    for_changed(lhs_delta, update);
    for_changed(rhs_delta, update);
}

inline void initialize_set(NegationTag,
                           DenotationState<ConceptTag>& output,
                           BorrowedDenotationView<ConceptTag> input,
                           const DenotationDelta<ConceptTag>&,
                           SetOperationWorkspace&)
{
    output.assign(input);
    output.flip();
}

inline void update_set(NegationTag,
                       DenotationState<ConceptTag>& output,
                       BorrowedDenotationView<ConceptTag> input,
                       const DenotationDelta<ConceptTag>& delta,
                       SetOperationWorkspace&)
{
    output.clear_delta();
    for_changed(delta, [&](auto value) { output.set(value, !contains(input, value)); });
}

inline void
initialize_set(ComplementTag, DenotationState<RoleTag>& output, BorrowedDenotationView<RoleTag> input, const DenotationDelta<RoleTag>&, SetOperationWorkspace&)
{
    output.assign(input);
    output.flip();
}

inline void update_set(ComplementTag,
                       DenotationState<RoleTag>& output,
                       BorrowedDenotationView<RoleTag> input,
                       const DenotationDelta<RoleTag>& delta,
                       SetOperationWorkspace&)
{
    output.clear_delta();
    for_changed(delta, [&](auto value) { output.set(value, !contains(input, value)); });
}

inline void
initialize_set(InverseTag, DenotationState<RoleTag>& output, BorrowedDenotationView<RoleTag> input, const DenotationDelta<RoleTag>&, SetOperationWorkspace&)
{
    output.initialize(input.get_num_objects());
    for (const auto edge : input)
        output.set({ edge.second, edge.first }, true);
    output.clear_delta();
}

inline void
update_set(InverseTag, DenotationState<RoleTag>& output, BorrowedDenotationView<RoleTag> input, const DenotationDelta<RoleTag>& delta, SetOperationWorkspace&)
{
    output.clear_delta();
    for_changed(delta, [&](auto edge) { output.set({ edge.second, edge.first }, contains(input, edge)); });
}

inline void initialize_set(IdentityTag,
                           DenotationState<RoleTag>& output,
                           BorrowedDenotationView<ConceptTag> input,
                           const DenotationDelta<ConceptTag>&,
                           SetOperationWorkspace&)
{
    output.initialize(input.get_num_objects());
    for (const auto object : input)
        output.set({ object, object }, true);
    output.clear_delta();
}

inline void update_set(IdentityTag,
                       DenotationState<RoleTag>& output,
                       BorrowedDenotationView<ConceptTag> input,
                       const DenotationDelta<ConceptTag>& delta,
                       SetOperationWorkspace&)
{
    output.clear_delta();
    for_changed(delta, [&](auto object) { output.set({ object, object }, contains(input, object)); });
}

inline void initialize_set(RestrictionTag,
                           DenotationState<RoleTag>& output,
                           BorrowedDenotationView<RoleTag> role,
                           const DenotationDelta<RoleTag>&,
                           BorrowedDenotationView<ConceptTag> concept_,
                           const DenotationDelta<ConceptTag>&,
                           SetOperationWorkspace& workspace)
{
    output.initialize(role.get_num_objects());
    workspace.mark_all(role.get_formalism_repository());
    evaluate_rows(RestrictionTag {}, output, role, concept_, workspace);
    output.clear_delta();
}

inline void update_set(RestrictionTag,
                       DenotationState<RoleTag>& output,
                       BorrowedDenotationView<RoleTag> role,
                       const DenotationDelta<RoleTag>& role_delta,
                       BorrowedDenotationView<ConceptTag> concept_,
                       const DenotationDelta<ConceptTag>& concept_delta,
                       SetOperationWorkspace& workspace)
{
    output.clear_delta();
    workspace.clear_rows();
    mark_sources(workspace, role_delta);
    mark_predecessors(workspace, role, concept_delta);
    evaluate_rows(RestrictionTag {}, output, role, concept_, workspace);
}

template<BaseConceptConstructorTag Tag>
void initialize_set(Tag,
                    DenotationState<ConceptTag>& output,
                    BorrowedDenotationView<RoleTag> role,
                    const DenotationDelta<RoleTag>&,
                    BorrowedDenotationView<ConceptTag> concept_,
                    const DenotationDelta<ConceptTag>&,
                    SetOperationWorkspace& workspace)
{
    output.initialize(role.get_num_objects());
    workspace.mark_all(role.get_formalism_repository());
    evaluate_rows(Tag {}, output, role, concept_, workspace);
    output.clear_delta();
}

template<BaseConceptConstructorTag Tag>
void update_set(Tag,
                DenotationState<ConceptTag>& output,
                BorrowedDenotationView<RoleTag> role,
                const DenotationDelta<RoleTag>& role_delta,
                BorrowedDenotationView<ConceptTag> concept_,
                const DenotationDelta<ConceptTag>& concept_delta,
                SetOperationWorkspace& workspace)
{
    output.clear_delta();
    workspace.clear_rows();
    mark_sources(workspace, role_delta);
    mark_predecessors(workspace, role, concept_delta);
    evaluate_rows(Tag {}, output, role, concept_, workspace);
}

template<BaseConceptConstructorTag Tag>
void initialize_set(Tag,
                    DenotationState<ConceptTag>& output,
                    BorrowedDenotationView<RoleTag> role,
                    const DenotationDelta<RoleTag>&,
                    ygg::uint_t threshold,
                    SetOperationWorkspace& workspace)
{
    output.initialize(role.get_num_objects());
    workspace.mark_all(role.get_formalism_repository());
    evaluate_rows(Tag {}, output, role, threshold, workspace);
    output.clear_delta();
}

template<BaseConceptConstructorTag Tag>
void update_set(Tag,
                DenotationState<ConceptTag>& output,
                BorrowedDenotationView<RoleTag> role,
                const DenotationDelta<RoleTag>& delta,
                ygg::uint_t threshold,
                SetOperationWorkspace& workspace)
{
    output.clear_delta();
    workspace.clear_rows();
    mark_sources(workspace, delta);
    evaluate_rows(Tag {}, output, role, threshold, workspace);
}

template<BaseConceptConstructorTag Tag>
void initialize_set(Tag,
                    DenotationState<ConceptTag>& output,
                    BorrowedDenotationView<RoleTag> role,
                    const DenotationDelta<RoleTag>&,
                    BorrowedDenotationView<ConceptTag> concept_,
                    const DenotationDelta<ConceptTag>&,
                    ygg::uint_t threshold,
                    SetOperationWorkspace& workspace)
{
    output.initialize(role.get_num_objects());
    workspace.mark_all(role.get_formalism_repository());
    evaluate_rows(Tag {}, output, role, concept_, threshold, workspace);
    output.clear_delta();
}

template<BaseConceptConstructorTag Tag>
void update_set(Tag,
                DenotationState<ConceptTag>& output,
                BorrowedDenotationView<RoleTag> role,
                const DenotationDelta<RoleTag>& role_delta,
                BorrowedDenotationView<ConceptTag> concept_,
                const DenotationDelta<ConceptTag>& concept_delta,
                ygg::uint_t threshold,
                SetOperationWorkspace& workspace)
{
    output.clear_delta();
    workspace.clear_rows();
    mark_sources(workspace, role_delta);
    mark_predecessors(workspace, role, concept_delta);
    evaluate_rows(Tag {}, output, role, concept_, threshold, workspace);
}

template<BaseConceptConstructorTag Tag>
void initialize_set(Tag,
                    DenotationState<ConceptTag>& output,
                    BorrowedDenotationView<RoleTag> lhs,
                    const DenotationDelta<RoleTag>&,
                    BorrowedDenotationView<RoleTag> rhs,
                    const DenotationDelta<RoleTag>&,
                    SetOperationWorkspace& workspace)
{
    output.initialize(lhs.get_num_objects());
    workspace.mark_all(lhs.get_formalism_repository());
    evaluate_rows(Tag {}, output, lhs, rhs, workspace);
    output.clear_delta();
}

template<BaseConceptConstructorTag Tag>
void update_set(Tag,
                DenotationState<ConceptTag>& output,
                BorrowedDenotationView<RoleTag> lhs,
                const DenotationDelta<RoleTag>& lhs_delta,
                BorrowedDenotationView<RoleTag> rhs,
                const DenotationDelta<RoleTag>& rhs_delta,
                SetOperationWorkspace& workspace)
{
    output.clear_delta();
    workspace.clear_rows();
    mark_sources(workspace, lhs_delta);
    mark_sources(workspace, rhs_delta);
    evaluate_rows(Tag {}, output, lhs, rhs, workspace);
}

template<ygg::SizedForwardRangeOf<tyr::formalism::planning::ObjectView> Objects>
void initialize_set(RoleFillersTag,
                    DenotationState<ConceptTag>& output,
                    BorrowedDenotationView<RoleTag> role,
                    const DenotationDelta<RoleTag>&,
                    const Objects& fillers,
                    SetOperationWorkspace& workspace)
{
    output.initialize(role.get_num_objects());
    workspace.mark_all(role.get_formalism_repository());
    evaluate_rows(RoleFillersTag {}, output, role, fillers, workspace);
    output.clear_delta();
}

template<ygg::SizedForwardRangeOf<tyr::formalism::planning::ObjectView> Objects>
void update_set(RoleFillersTag,
                DenotationState<ConceptTag>& output,
                BorrowedDenotationView<RoleTag> role,
                const DenotationDelta<RoleTag>& delta,
                const Objects& fillers,
                SetOperationWorkspace& workspace)
{
    output.clear_delta();
    workspace.clear_rows();
    mark_sources(workspace, delta);
    evaluate_rows(RoleFillersTag {}, output, role, fillers, workspace);
}

inline void initialize_set(CompositionTag,
                           DenotationState<RoleTag>& output,
                           BorrowedDenotationView<RoleTag> lhs,
                           const DenotationDelta<RoleTag>&,
                           BorrowedDenotationView<RoleTag> rhs,
                           const DenotationDelta<RoleTag>&,
                           SetOperationWorkspace& workspace)
{
    output.initialize(lhs.get_num_objects());
    workspace.mark_all(lhs.get_formalism_repository());
    evaluate_rows(CompositionTag {}, output, lhs, rhs, workspace);
    output.clear_delta();
}

inline void update_set(CompositionTag,
                       DenotationState<RoleTag>& output,
                       BorrowedDenotationView<RoleTag> lhs,
                       const DenotationDelta<RoleTag>& lhs_delta,
                       BorrowedDenotationView<RoleTag> rhs,
                       const DenotationDelta<RoleTag>& rhs_delta,
                       SetOperationWorkspace& workspace)
{
    output.clear_delta();
    workspace.clear_rows();
    mark_sources(workspace, lhs_delta);
    for_changed(rhs_delta, [&](auto edge) { mark_predecessors(workspace, lhs, edge.first); });
    // ponytail: affected rows are rebuilt; retain witness counts if dense rows dominate profiling.
    evaluate_rows(CompositionTag {}, output, lhs, rhs, workspace);
}

template<BaseRoleConstructorTag Tag>
void initialize_set(Tag,
                    DenotationState<RoleTag>& output,
                    BorrowedDenotationView<RoleTag> role,
                    const DenotationDelta<RoleTag>&,
                    SetOperationWorkspace& workspace)
{
    output.initialize(role.get_num_objects());
    workspace.mark_all(role.get_formalism_repository());
    evaluate_rows(Tag {}, output, role, workspace);
    output.clear_delta();
}

template<BaseRoleConstructorTag Tag>
void update_set(Tag,
                DenotationState<RoleTag>& output,
                BorrowedDenotationView<RoleTag> role,
                const DenotationDelta<RoleTag>& delta,
                SetOperationWorkspace& workspace)
{
    output.clear_delta();
    workspace.clear_rows();
    // Read all dirty predecessors from the old closure before modifying it.
    for_changed(delta,
                [&](auto edge)
                {
                    workspace.mark(edge.first);
                    mark_predecessors(workspace, output.get_result(role.get_formalism_repository()), edge.first);
                });
    evaluate_rows(Tag {}, output, role, workspace);
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
