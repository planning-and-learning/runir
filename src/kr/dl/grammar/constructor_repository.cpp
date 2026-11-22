#include "runir/kr/dl/grammar/constructor_repository.hpp"


#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>


// The repository's symbols, which the library's other sources only declare (yggdrasil/formalism/symbol_context.hpp).
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::dl::grammar::ConstructorRepository, 58)
YGG_FORMALISM_BUILDER(, runir::kr::dl::grammar::Builder, 58)
