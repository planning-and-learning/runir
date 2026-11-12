#ifndef RUNIR_KR_DL_SEMANTICS_DATALOG_FEATURE_HPP_
#define RUNIR_KR_DL_SEMANTICS_DATALOG_FEATURE_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <tyr/datalog/lifted/database/datalog/evaluator.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::dl::semantics
{

/// A program's evaluator in full evaluation, over the heads that features read; `generation` counts its evaluations.
struct DatalogFeature
{
    tyr::datalog::database::datalog::Evaluator evaluator;
    urd::Store<urd::sequence<urd::index<tyr::datalog::database::datalog::Relation>>> heads;
    uint64_t generation = 0;

    /// The output of a head relation.
    [[nodiscard]] size_t output(urd::index<tyr::datalog::database::datalog::Relation> relation) const
    {
        const auto it = std::ranges::find(heads, relation);
        if (it == heads.end())
            throw std::logic_error("q_head: the relation is not a head of its program.");
        return static_cast<size_t>(it - heads.begin());
    }
};

/// The heads that features read, the evaluator's outputs. The repository is a template parameter so that the body is
/// checked once it is complete.
template<typename C>
urd::Store<urd::sequence<tyr::datalog::database::datalog::RelationView>> program_heads(urd::index_view<DatalogProgram, C> program)
{
    auto heads = urd::Store<urd::sequence<tyr::datalog::database::datalog::RelationView>>();
    for (const auto head : program.get_outputs())
        heads.push_back(head);
    return heads;
}

/// The input slots whose queries are static.
template<typename C>
urd::Store<urd::sequence<size_t>> static_slots(urd::index_view<DatalogProgram, C> program)
{
    auto result = urd::Store<urd::sequence<size_t>>();
    const auto inputs = program.get_inputs();
    for (size_t slot = 0; slot < inputs.size(); ++slot)
        if (inputs[slot].is_static())
            result.push_back(slot);
    return result;
}

/// The program's evaluator, whose results are read through `layer`.
template<typename C>
DatalogFeature make_datalog_feature(const ygg::FreeRelationRepository& layer, urd::index_view<DatalogProgram, C> program)
{
    const auto heads = program_heads(program);
    const auto slots = static_slots(program);
    auto feature = DatalogFeature { tyr::datalog::database::datalog::Evaluator(layer, program.get_program(), heads, slots), {}, 0 };
    for (const auto head : heads)
        feature.heads.push_back(head.index());
    return feature;
}

}  // namespace runir::kr::dl::semantics

#endif
