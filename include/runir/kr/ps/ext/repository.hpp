#ifndef RUNIR_KR_PS_EXT_REPOSITORY_HPP_
#define RUNIR_KR_PS_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/dl/condition_builder.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_builder.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_builder.hpp"
#include "runir/kr/ps/dl/feature_data.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/ext/canonicalization.hpp"
#include "runir/kr/ps/ext/module_builder.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/order_term_builder.hpp"
#include "runir/kr/ps/ext/program_builder.hpp"
#include "runir/kr/ps/ext/program_data.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/rule_builder.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/kr/ps/feature_builder.hpp"
#include "runir/kr/ps/feature_data.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/memory_state_builder.hpp"
#include "runir/kr/ps/memory_state_data.hpp"
#include "runir/kr/ps/memory_state_view.hpp"
#include "runir/kr/ps/module_symbol_builder.hpp"
#include "runir/kr/ps/module_symbol_data.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"
#include "runir/kr/ps/repository.hpp"
#include "runir/kr/ps/rule_variant_builder.hpp"
#include "runir/kr/ps/rule_variant_data.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <utility>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::kr::ps::ext
{

/// The family's records, in a class of its own (named, so that names that mention it spell its name, not its types).
class Repository final : public runir::kr::ps::BasicRepository<Repository, runir::kr::ExtFamilyTag, RepositoryTypes>
{
public:
    explicit Repository(DlRepositoryPtr dl_repository) :
        runir::kr::ps::BasicRepository<Repository, runir::kr::ExtFamilyTag, RepositoryTypes>(std::move(dl_repository))
    {
    }
};

/// Its builder types under a name of their own (yggdrasil's NamesList).
struct Builders : boost::mp11::mp_apply<ygg::formalism::Builders, RepositoryTypes>
{
};

using Builder = ygg::formalism::BasicBuilderStorage<Repository, Builders>;

using ygg::formalism::checkout;

}

// Its symbols' reads, finds and inserts instantiated once, where the library instantiates the repository (brokkr's
// include rule I7): a file that reads records through it completes none of its stores.
YGG_FORMALISM_SYMBOL_CONTEXT(extern, runir::kr::ps::ext::Repository, 51)

#endif
