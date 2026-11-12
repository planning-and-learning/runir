#ifndef RUNIR_SERIALIZATION_TYPES_DL_HPP_
#define RUNIR_SERIALIZATION_TYPES_DL_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/serialization/types/views.hpp"

namespace runir::serialization
{

using DenotationViews = boost::mp11::mp_append<boost::mp11::mp_transform<kr::dl::semantics::DenotationView, kr::dl::CategoryTags>,
                                               boost::mp11::mp_list<kr::dl::semantics::RegisterValuesView, kr::dl::semantics::CallArgumentsView>>;

}  // namespace runir::serialization

#endif
