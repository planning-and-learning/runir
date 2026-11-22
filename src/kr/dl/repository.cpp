#include "runir/kr/dl/repository.hpp"


#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>


// The repository's symbols, which the library's other sources only declare (yggdrasil/formalism/symbol_context.hpp).
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::dl::ConstructorRepository, 114)
YGG_FORMALISM_BUILDER(, runir::kr::dl::Builder, 114)
