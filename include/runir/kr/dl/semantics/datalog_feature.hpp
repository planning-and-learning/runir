#ifndef RUNIR_KR_DL_SEMANTICS_DATALOG_FEATURE_HPP_
#define RUNIR_KR_DL_SEMANTICS_DATALOG_FEATURE_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <yggdrasil/database/datalog/evaluator.hpp>

namespace runir::kr::dl::semantics
{

/// A program's evaluator in full evaluation, over all heads of its rules; `generation` counts its evaluations.
struct DatalogFeature
{
    ygg::database::datalog::Evaluator evaluator;
    std::vector<ygg::Index<ygg::database::datalog::Relation>> heads;
    uint64_t generation = 0;

    /// The output of a head relation.
    [[nodiscard]] size_t output(ygg::Index<ygg::database::datalog::Relation> relation) const
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
std::vector<ygg::database::datalog::RelationView> program_heads(ygg::View<ygg::Index<DatalogProgram>, C> program)
{
    auto heads = std::vector<ygg::database::datalog::RelationView>();
    for (const auto head : program.get_outputs())
        heads.push_back(head);
    return heads;
}

/// The input slots whose queries are static.
template<typename C>
std::vector<size_t> static_slots(ygg::View<ygg::Index<DatalogProgram>, C> program)
{
    auto result = std::vector<size_t>();
    const auto inputs = program.get_inputs();
    for (size_t slot = 0; slot < inputs.size(); ++slot)
        if (inputs[slot].is_static())
            result.push_back(slot);
    return result;
}

template<typename C>
DatalogFeature make_datalog_feature(ygg::View<ygg::Index<DatalogProgram>, C> program)
{
    const auto heads = program_heads(program);
    const auto slots = static_slots(program);
    auto feature = DatalogFeature { ygg::database::datalog::Evaluator(program.get_program(), heads, slots), {}, 0 };
    for (const auto head : heads)
        feature.heads.push_back(head.get_index());
    return feature;
}

}  // namespace runir::kr::dl::semantics

#endif
