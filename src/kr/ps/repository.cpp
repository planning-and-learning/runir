#include "runir/kr/ps/base/repository.hpp"

#include "runir/kr/ps/detail/repository_instantiations.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/icp/execution_data.hpp"
#include "runir/kr/ps/icp/repository.hpp"
#include "runir/kr/uns/repository.hpp"
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>
#include <yggdrasil/formalism/builder_instantiations.hpp>

RUNIR_KR_PS_INTERNS()

// The symbols and builders of the library's repositories, which its other sources only declare.
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::base::Repository, 16)
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::ext::Repository, 51)
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::icp::Repository, 30)
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::uns::Repository, 5)
YGG_FORMALISM_BUILDER(, runir::kr::ps::base::Builder, 16)
YGG_FORMALISM_BUILDER(, runir::kr::ps::ext::Builder, 51)
YGG_FORMALISM_BUILDER(, runir::kr::ps::icp::Builder, 30)
YGG_FORMALISM_BUILDER(, runir::kr::uns::Builder, 5)
