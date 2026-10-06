#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_SET_OPERATIONS_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_SET_OPERATIONS_HPP_

#include "runir/kr/dl/semantics/incremental/detail/closure.hpp"
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
    std::vector<ygg::Index<tyr::formalism::Object>> rows;
    std::vector<std::uint8_t> marked;
    ygg::Builder<Denotation<ConceptTag>> row;
    ClosureWorkspace closure;

    void initialize(ygg::uint_t num_objects)
    {
        rows.clear();
        rows.reserve(num_objects);
        marked.assign(num_objects, 0);
        row.initialize(num_objects);
        closure.initialize(num_objects);
    }

    void mark(ygg::Index<tyr::formalism::Object> source)
    {
        const auto index = ygg::uint_t(source);
        if (!marked[index])
        {
            rows.push_back(source);
            marked[index] = 1;
        }
    }

    void clear_rows() noexcept
    {
        for (const auto source : rows)
            marked[ygg::uint_t(source)] = 0;
        rows.clear();
    }

    void mark_all()
    {
        clear_rows();
        for (ygg::uint_t source = 0; source < marked.size(); ++source)
            mark(ygg::Index<tyr::formalism::Object>(source));
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
    for_changed(delta, [&](auto edge) { workspace.mark(edge.first.get_index()); });
}

inline void mark_predecessors(SetOperationWorkspace& workspace, const ygg::Builder<Denotation<RoleTag>>& role, ygg::Index<tyr::formalism::Object> target)
{
    const auto index = ygg::uint_t(target);
    for (ygg::uint_t source = 0; source < role.get_num_objects(); ++source)
        if (role.get(source).test(index))
            workspace.mark(ygg::Index<tyr::formalism::Object>(source));
}

inline void mark_predecessors(SetOperationWorkspace& workspace, const ygg::Builder<Denotation<RoleTag>>& role, const DenotationDelta<ConceptTag>& delta)
{
    for_changed(delta, [&](auto target) { mark_predecessors(workspace, role, target.get_index()); });
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

template<std::invocable<ygg::Index<tyr::formalism::Object>, ygg::BitsetSpan<const ygg::uint_t>> Emit>
void evaluate_rows(RestrictionTag,
                   BorrowedDenotationView<RoleTag> role,
                   BorrowedDenotationView<ConceptTag> concept_,
                   SetOperationWorkspace& workspace,
                   Emit emit)
{
    for (const auto source : workspace.rows)
    {
        auto row = workspace.row.get();
        row.copy_from(role.get(source));
        row &= concept_.get();
        emit(source, ygg::BitsetSpan<const ygg::uint_t>(row));
    }
}

template<BaseConceptConstructorTag Tag, std::invocable<ygg::Index<tyr::formalism::Object>, bool> Emit>
void evaluate_rows(Tag, BorrowedDenotationView<RoleTag> role, BorrowedDenotationView<ConceptTag> concept_, SetOperationWorkspace& workspace, Emit emit)
{
    static_assert(std::same_as<Tag, ValueRestrictionTag> || std::same_as<Tag, ExistentialQuantificationTag>);
    for (const auto source : workspace.rows)
    {
        const auto present =
            std::same_as<Tag, ValueRestrictionTag> ? role.get(source).is_subset_of(concept_.get()) : role.get(source).intersects(concept_.get());
        emit(source, present);
    }
}

template<BaseConceptConstructorTag Tag, std::invocable<ygg::Index<tyr::formalism::Object>, bool> Emit>
void evaluate_rows(Tag, BorrowedDenotationView<RoleTag> role, ygg::uint_t threshold, SetOperationWorkspace& workspace, Emit emit)
{
    for (const auto source : workspace.rows)
        emit(source, number_restriction<Tag>(role.get(source).count(), threshold));
}

template<BaseConceptConstructorTag Tag, std::invocable<ygg::Index<tyr::formalism::Object>, bool> Emit>
void evaluate_rows(Tag,
                   BorrowedDenotationView<RoleTag> role,
                   BorrowedDenotationView<ConceptTag> concept_,
                   ygg::uint_t threshold,
                   SetOperationWorkspace& workspace,
                   Emit emit)
{
    for (const auto source : workspace.rows)
    {
        const auto count = role.get(source).count_intersection(concept_.get());
        emit(source, number_restriction<Tag>(count, threshold));
    }
}

template<BaseConceptConstructorTag Tag, std::invocable<ygg::Index<tyr::formalism::Object>, bool> Emit>
void evaluate_rows(Tag, BorrowedDenotationView<RoleTag> lhs, BorrowedDenotationView<RoleTag> rhs, SetOperationWorkspace& workspace, Emit emit)
{
    static_assert(std::same_as<Tag, RoleValueMapTag> || std::same_as<Tag, AgreementTag>);
    for (const auto source : workspace.rows)
    {
        const auto present = std::same_as<Tag, RoleValueMapTag> ? lhs.get(source).is_subset_of(rhs.get(source)) : lhs.get(source) == rhs.get(source);
        emit(source, present);
    }
}

template<ygg::SizedForwardRangeOf<tyr::formalism::planning::ObjectView> Objects, std::invocable<ygg::Index<tyr::formalism::Object>, bool> Emit>
void evaluate_rows(RoleFillersTag, BorrowedDenotationView<RoleTag> role, const Objects& fillers, SetOperationWorkspace& workspace, Emit emit)
{
    for (const auto source : workspace.rows)
    {
        bool present = true;
        for (const auto filler : fillers)
            if (!role.get(source).test(ygg::uint_t(filler.get_index())))
            {
                present = false;
                break;
            }
        emit(source, present);
    }
}

template<std::invocable<ygg::Index<tyr::formalism::Object>, ygg::BitsetSpan<const ygg::uint_t>> Emit>
void evaluate_rows(CompositionTag, BorrowedDenotationView<RoleTag> lhs, BorrowedDenotationView<RoleTag> rhs, SetOperationWorkspace& workspace, Emit emit)
{
    for (const auto source : workspace.rows)
    {
        auto row = workspace.row.get();
        row.reset();
        for (const auto middle : ygg::set_bit_indices(lhs.get(source)))
            row |= rhs.get(static_cast<ygg::uint_t>(middle));
        emit(source, ygg::BitsetSpan<const ygg::uint_t>(row));
    }
}

template<BaseRoleConstructorTag Tag, std::invocable<ygg::Index<tyr::formalism::Object>, ygg::BitsetSpan<const ygg::uint_t>> Emit>
void evaluate_rows(Tag, const ygg::Builder<Denotation<RoleTag>>& result, BorrowedDenotationView<RoleTag> role, SetOperationWorkspace& workspace, Emit emit)
{
    evaluate_closure(Tag {}, result, role.get_handle(), workspace.rows, workspace.marked, workspace.row, workspace.closure, emit);
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
    auto& result = output.initialize(lhs.get_num_objects());
    auto bits = result.storage_bits();
    bits.copy_from(lhs.storage_bits());
    bits &= rhs.storage_bits();
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
    auto& result = output.initialize(lhs.get_num_objects());
    auto bits = result.storage_bits();
    bits.copy_from(lhs.storage_bits());
    bits |= rhs.storage_bits();
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
    output.assign(input).flip();
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
    output.assign(input).flip();
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
    auto& result = output.initialize(input.get_num_objects());
    for (const auto [source, target] : input.indices())
        result.set(target, source, true);
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
    auto& result = output.initialize(input.get_num_objects());
    for (const auto object : input.indices())
        result.set(object, object, true);
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
    auto& result = output.initialize(role.get_num_objects());
    workspace.mark_all();
    evaluate_rows(RestrictionTag {}, role, concept_, workspace, [&](auto source, auto row) { result.assign_row(source, row); });
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
    mark_predecessors(workspace, role.get_handle(), concept_delta);
    evaluate_rows(RestrictionTag {},
                  role,
                  concept_,
                  workspace,
                  [&](auto source, auto row) { output.update_row(source, row, role.get_formalism_repository()); });
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
    auto& result = output.initialize(role.get_num_objects());
    workspace.mark_all();
    evaluate_rows(Tag {}, role, concept_, workspace, [&](auto source, bool present) { result.set(source, present); });
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
    mark_predecessors(workspace, role.get_handle(), concept_delta);
    evaluate_rows(Tag {}, role, concept_, workspace, [&](auto source, bool present) { output.set(source, present, role.get_formalism_repository()); });
}

template<BaseConceptConstructorTag Tag>
void initialize_set(Tag,
                    DenotationState<ConceptTag>& output,
                    BorrowedDenotationView<RoleTag> role,
                    const DenotationDelta<RoleTag>&,
                    ygg::uint_t threshold,
                    SetOperationWorkspace& workspace)
{
    auto& result = output.initialize(role.get_num_objects());
    workspace.mark_all();
    evaluate_rows(Tag {}, role, threshold, workspace, [&](auto source, bool present) { result.set(source, present); });
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
    evaluate_rows(Tag {}, role, threshold, workspace, [&](auto source, bool present) { output.set(source, present, role.get_formalism_repository()); });
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
    auto& result = output.initialize(role.get_num_objects());
    workspace.mark_all();
    evaluate_rows(Tag {}, role, concept_, threshold, workspace, [&](auto source, bool present) { result.set(source, present); });
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
    mark_predecessors(workspace, role.get_handle(), concept_delta);
    evaluate_rows(Tag {},
                  role,
                  concept_,
                  threshold,
                  workspace,
                  [&](auto source, bool present) { output.set(source, present, role.get_formalism_repository()); });
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
    auto& result = output.initialize(lhs.get_num_objects());
    workspace.mark_all();
    evaluate_rows(Tag {}, lhs, rhs, workspace, [&](auto source, bool present) { result.set(source, present); });
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
    evaluate_rows(Tag {}, lhs, rhs, workspace, [&](auto source, bool present) { output.set(source, present, lhs.get_formalism_repository()); });
}

template<ygg::SizedForwardRangeOf<tyr::formalism::planning::ObjectView> Objects>
void initialize_set(RoleFillersTag,
                    DenotationState<ConceptTag>& output,
                    BorrowedDenotationView<RoleTag> role,
                    const DenotationDelta<RoleTag>&,
                    const Objects& fillers,
                    SetOperationWorkspace& workspace)
{
    auto& result = output.initialize(role.get_num_objects());
    workspace.mark_all();
    evaluate_rows(RoleFillersTag {}, role, fillers, workspace, [&](auto source, bool present) { result.set(source, present); });
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
    evaluate_rows(RoleFillersTag {},
                  role,
                  fillers,
                  workspace,
                  [&](auto source, bool present) { output.set(source, present, role.get_formalism_repository()); });
}

inline void initialize_set(CompositionTag,
                           DenotationState<RoleTag>& output,
                           BorrowedDenotationView<RoleTag> lhs,
                           const DenotationDelta<RoleTag>&,
                           BorrowedDenotationView<RoleTag> rhs,
                           const DenotationDelta<RoleTag>&,
                           SetOperationWorkspace& workspace)
{
    auto& result = output.initialize(lhs.get_num_objects());
    workspace.mark_all();
    evaluate_rows(CompositionTag {}, lhs, rhs, workspace, [&](auto source, auto row) { result.assign_row(source, row); });
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
    for_changed(rhs_delta, [&](auto edge) { mark_predecessors(workspace, lhs.get_handle(), edge.first.get_index()); });
    // ponytail: affected rows are rebuilt; retain witness counts if dense rows dominate profiling.
    evaluate_rows(CompositionTag {}, lhs, rhs, workspace, [&](auto source, auto row) { output.update_row(source, row, lhs.get_formalism_repository()); });
}

template<BaseRoleConstructorTag Tag>
void initialize_set(Tag,
                    DenotationState<RoleTag>& output,
                    BorrowedDenotationView<RoleTag> role,
                    const DenotationDelta<RoleTag>&,
                    SetOperationWorkspace& workspace)
{
    auto& result = output.initialize(role.get_num_objects());
    workspace.mark_all();
    evaluate_rows(Tag {}, result, role, workspace, [&](auto source, auto row) { result.assign_row(source, row); });
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
                    workspace.mark(edge.first.get_index());
                    mark_predecessors(workspace, output.get_builder(), edge.first.get_index());
                });
    evaluate_rows(Tag {},
                  output.get_builder(),
                  role,
                  workspace,
                  [&](auto source, auto row) { output.update_row(source, row, role.get_formalism_repository()); });
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
