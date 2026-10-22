#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/detail/constructor_repository_impl.hpp"

namespace runir::kr::dl::detail
{

template class ConstructorRepositoryFactory<runir::kr::dl::FamilyConstructorRepositoryTypes>;

}  // namespace runir::kr::dl::detail
