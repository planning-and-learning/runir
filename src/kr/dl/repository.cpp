#include "runir/kr/dl/repository.hpp"

#include "runir/kr/dl/detail/repository_instantiations.hpp"

#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

RUNIR_KR_DL_INTERNS()

// The repository's symbols, which the library's other sources only declare (yggdrasil/formalism/symbol_context.hpp).
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::dl::ConstructorRepository, 110)
YGG_FORMALISM_BUILDER(, runir::kr::dl::Builder, 110)
