#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/detail/constructor_repository_impl.hpp"

namespace runir::kr::dl::detail
{

template class BasicConstructorRepositoryFactory<runir::kr::dl::ConstructorRepositoryTypes>;

}  // namespace runir::kr::dl::detail
