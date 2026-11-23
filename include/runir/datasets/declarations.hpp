#ifndef RUNIR_DATASETS_DECLARATIONS_HPP_
#define RUNIR_DATASETS_DECLARATIONS_HPP_

#include <string_view>
#include <tyr/declarations.hpp>
#include <urd/elements/concepts.hpp>

namespace runir::datasets
{

/// The labels of the state and equivalence graphs' vertices and edges (records: data<...>, the graph's elements).
template<tyr::TaskKind Kind>
struct StateGraphVertexLabel
{
    static constexpr std::string_view name = urd::joined_name<"StateGraphVertexLabel", Kind::name>;
};

template<tyr::TaskKind Kind>
struct AnnotatedStateGraphVertexLabel
{
    static constexpr std::string_view name = urd::joined_name<"AnnotatedStateGraphVertexLabel", Kind::name>;
};

struct StateGraphEdgeLabel
{
    static constexpr std::string_view name = "StateGraphEdgeLabel";
};

struct EquivalenceVertexLabel
{
    static constexpr std::string_view name = "EquivalenceVertexLabel";
};

struct AnnotatedEquivalenceVertexLabel
{
    static constexpr std::string_view name = "AnnotatedEquivalenceVertexLabel";
};

struct EquivalenceEdgeLabel
{
    static constexpr std::string_view name = "EquivalenceEdgeLabel";
};

}  // namespace runir::datasets

#endif
