#ifndef RUNIR_SERIALIZATION_TYPES_UNS_HPP_
#define RUNIR_SERIALIZATION_TYPES_UNS_HPP_

#include "runir/kr/uns/repository.hpp"
#include "runir/serialization/types/views.hpp"

namespace runir::serialization
{

using UnsViews = boost::mp11::mp_transform_q<boost::mp11::mp_bind_front<IndexView, kr::uns::Repository>, kr::uns::RepositoryTypes>;

}  // namespace runir::serialization

#endif
