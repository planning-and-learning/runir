#include "runir/kr/dl/grammar/constructor_repository.hpp"

namespace runir::kr::dl::grammar
{

template class BasicConstructorRepositoryFactory<runir::kr::BaseFamilyTag>;
template class BasicConstructorRepositoryFactory<runir::kr::ExtFamilyTag>;
template class BasicConstructorRepositoryFactory<runir::kr::UnsFamilyTag>;

}  // namespace runir::kr::dl::grammar
