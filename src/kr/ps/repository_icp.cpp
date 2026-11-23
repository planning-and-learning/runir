#include "runir/kr/ps/icp/execution_data.hpp"
#include "runir/kr/ps/icp/repository.hpp"

#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_impl.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

// The symbols and builders of the icp family's repository (ps/repository.cpp).
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::icp::Repository, 30)
YGG_FORMALISM_BUILDER(, runir::kr::ps::icp::Builder, 30)
