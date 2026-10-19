#ifndef RUNIR_SERIALIZATION_TYPES_DL_HPP_
#define RUNIR_SERIALIZATION_TYPES_DL_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/serialization/types/views.hpp"

namespace runir::serialization
{

using DenotationViews = ygg::ConcatTypeListsT<ygg::MapTypeListT<kr::dl::semantics::DenotationView, kr::dl::CategoryTags>,
                                              ygg::TypeList<kr::dl::semantics::RegisterValuesView, kr::dl::semantics::CallArgumentsView>>;

}  // namespace runir::serialization

#endif
