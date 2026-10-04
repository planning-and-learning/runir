#include "runir/kr/dl/grammar/constructor_repository.hpp"

namespace runir::kr::dl::detail
{

template class ConstructorRepositoryFactory<runir::kr::BaseFamilyTag, runir::kr::dl::grammar::FamilyConstructorRepositoryTypes<runir::kr::BaseFamilyTag>>;
template class ConstructorRepositoryFactory<runir::kr::ExtFamilyTag, runir::kr::dl::grammar::FamilyConstructorRepositoryTypes<runir::kr::ExtFamilyTag>>;
template class ConstructorRepositoryFactory<runir::kr::UnsFamilyTag, runir::kr::dl::grammar::FamilyConstructorRepositoryTypes<runir::kr::UnsFamilyTag>>;

}  // namespace runir::kr::dl::detail
