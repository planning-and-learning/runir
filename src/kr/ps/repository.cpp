#include "runir/kr/ps/base/repository.hpp"

#include "runir/kr/uns/repository.hpp"

#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_impl.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

// The symbols and builders of the library's base and uns repositories, which its other sources only declare; the ext
// and icp families' in repository_ext.cpp and repository_icp.cpp, so that no one source holds them all (brokkr's
// template rule T5).
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::base::Repository, 16)
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::uns::Repository, 5)
YGG_FORMALISM_BUILDER(, runir::kr::ps::base::Builder, 16)
YGG_FORMALISM_BUILDER(, runir::kr::uns::Builder, 5)
