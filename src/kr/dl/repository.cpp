#include "runir/kr/dl/repository.hpp"

namespace runir::kr::dl
{

template class BasicConstructorRepositoryFactory<runir::kr::BaseFamilyTag>;
template class BasicConstructorRepositoryFactory<runir::kr::ExtFamilyTag>;
template class BasicConstructorRepositoryFactory<runir::kr::UnsFamilyTag>;

}  // namespace runir::kr::dl
