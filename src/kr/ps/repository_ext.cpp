#include "runir/kr/ps/ext/execution_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_impl.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

// The symbols and builders of the ext family's repository (ps/repository.cpp).
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::ext::Repository, 51)
YGG_FORMALISM_BUILDER(, runir::kr::ps::ext::Builder, 51)
