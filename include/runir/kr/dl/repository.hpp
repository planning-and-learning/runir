#ifndef RUNIR_REPOSITORY_HPP_
#define RUNIR_REPOSITORY_HPP_

#include "runir/kr/dl/argument_builder.hpp"
#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/boolean_builder.hpp"
#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/concept_builder.hpp"
#include "runir/kr/dl/construction_metadata.hpp"
#include "runir/kr/dl/constructor_builder.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/detail/constructor_repository.hpp"
#include "runir/kr/dl/numerical_builder.hpp"
#include "runir/kr/dl/query_builder.hpp"
#include "runir/kr/dl/register_builder.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/dl/role_builder.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"

#include <boost/mp11.hpp>
#include <memory>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::kr::dl
{

/// Its constructors, in a class of its own (named, so that names that mention it spell its name, not its types).
class ConstructorRepository final : public runir::kr::dl::detail::BasicConstructorRepository<ConstructorRepository, ConstructorRepositoryTypes>
{
public:
    explicit ConstructorRepository(std::shared_ptr<const tyr::formalism::Repository> planning_repository) :
        runir::kr::dl::detail::BasicConstructorRepository<ConstructorRepository, ConstructorRepositoryTypes>(std::move(planning_repository))
    {
    }
};

/// Its builder types under a name of their own (yggdrasil's NamesList).
struct Builders : boost::mp11::mp_apply<ygg::formalism::Builders, ConstructorRepositoryTypes>
{
};

using Builder = ygg::formalism::BasicBuilderStorage<ConstructorRepository, Builders>;

using ygg::formalism::checkout;

}

// Its symbols' reads, finds and inserts instantiated once, where the library instantiates the repository (brokkr's
// include rule I7): a file that reads records through it completes none of its stores.
YGG_FORMALISM_SYMBOL_CONTEXT(extern, runir::kr::dl::ConstructorRepository, 114)

#endif
