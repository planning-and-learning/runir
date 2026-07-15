#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"

namespace runir::kr::dl::cnf_grammar
{

template class BasicConstructorRepositoryFactory<runir::kr::BaseFamilyTag>;
template class BasicConstructorRepositoryFactory<runir::kr::ExtFamilyTag>;
template class BasicConstructorRepositoryFactory<runir::kr::UnsFamilyTag>;

}  // namespace runir::kr::dl::cnf_grammar
