#ifndef RUNIR_SERIALIZATION_TYPES_PS_ICP_HPP_
#define RUNIR_SERIALIZATION_TYPES_PS_ICP_HPP_

#include "runir/kr/ps/icp/repository.hpp"
#include "runir/serialization/types/views.hpp"

namespace runir::serialization
{

using IcpViews = ygg::MapTypeListSecondT<IndexView, kr::ps::icp::Repository, kr::ps::icp::RepositoryTypes>;

}  // namespace runir::serialization

#endif
