#include "pyrunir/kr/dl/serialization.hpp"

#include "module.hpp"

#include <boost/mp11.hpp>
#include <runir/datasets/graph_types.hpp>
#include <runir/datasets/state_graph.hpp>
#include <tyr/formalism/planning/formatter.hpp>
#include <tyr/planning/states/state_view.hpp>

namespace runir::datasets
{
void bind_serialization()
{
    ygg::python::bind_serialization(boost::mp11::mp_list<urd::data<StateGraphVertexLabel<tyr::GroundTag>>,
                                                         urd::data<StateGraphVertexLabel<tyr::LiftedTag>>,
                                                         urd::data<AnnotatedStateGraphVertexLabel<tyr::GroundTag>>,
                                                         urd::data<AnnotatedStateGraphVertexLabel<tyr::LiftedTag>>,
                                                         urd::data<StateGraphEdgeLabel>,
                                                         StateGraph<tyr::GroundTag>,
                                                         StateGraph<tyr::LiftedTag>,
                                                         AnnotatedStateGraph<tyr::GroundTag>,
                                                         AnnotatedStateGraph<tyr::LiftedTag>> {});
}
}  // namespace runir::datasets
