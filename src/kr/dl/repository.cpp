#include "runir/kr/dl/repository.hpp"

namespace runir::kr::dl::detail
{

template class ConstructorRepositoryFactory<runir::kr::BaseFamilyTag, runir::kr::dl::FamilyConstructorRepositoryTypes<runir::kr::BaseFamilyTag>>;
template class ConstructorRepositoryFactory<runir::kr::ExtFamilyTag, runir::kr::dl::FamilyConstructorRepositoryTypes<runir::kr::ExtFamilyTag>>;
template class ConstructorRepositoryFactory<runir::kr::UnsFamilyTag, runir::kr::dl::FamilyConstructorRepositoryTypes<runir::kr::UnsFamilyTag>>;

}  // namespace runir::kr::dl::detail
