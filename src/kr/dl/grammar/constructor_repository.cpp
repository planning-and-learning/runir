#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/detail/constructor_repository_impl.hpp"

namespace runir::kr::dl::detail
{

template class ConstructorRepositoryFactory<runir::kr::dl::grammar::FamilyConstructorRepositoryTypes>;

}  // namespace runir::kr::dl::detail
