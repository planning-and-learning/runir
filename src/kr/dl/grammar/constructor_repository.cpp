#include "runir/kr/dl/grammar/constructor_repository.hpp"

#include "runir/kr/dl/detail/constructor_repository_impl.hpp"
#include "runir/kr/dl/grammar/detail/repository_instantiations.hpp"

namespace runir::kr::dl::detail
{

template class BasicConstructorRepositoryFactory<runir::kr::dl::grammar::ConstructorRepositoryTypes>;

}  // namespace runir::kr::dl::detail

RUNIR_KR_DL_GRAMMAR_INTERNS()
