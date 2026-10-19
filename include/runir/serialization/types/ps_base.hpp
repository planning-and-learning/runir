#ifndef RUNIR_SERIALIZATION_TYPES_PS_BASE_HPP_
#define RUNIR_SERIALIZATION_TYPES_PS_BASE_HPP_

#include "runir/kr/ps/base/repository.hpp"
#include "runir/serialization/types/views.hpp"

namespace runir::serialization
{

using BaseViews = ygg::MapTypeListSecondT<IndexView, kr::ps::base::Repository, kr::ps::base::RepositoryTypes>;

}  // namespace runir::serialization

#endif
