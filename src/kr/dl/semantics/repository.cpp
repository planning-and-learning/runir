#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <cassert>
#include <memory>
#include <runir/kr/database/syntax/query_repository.hpp>
#include <utility>
#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_impl.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::kr::dl::semantics
{

DenotationRepository::DenotationRepository(std::shared_ptr<const tyr::formalism::Repository> formalism_repository) :
    Base(),
    NestsBase(std::move(formalism_repository)),
    m_relation_repository(),
    m_index(ygg::next_layer())
{ this->reset(); }

DenotationRepository::~DenotationRepository() = default;

}  // namespace runir::kr::dl::semantics

// The symbols and builders of the library's repositories, which its other sources only declare.
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::dl::semantics::DenotationRepository, 7)
YGG_FORMALISM_BUILDER(, runir::kr::dl::semantics::RecordStorage, 7)
YGG_FORMALISM_BUILDER(, runir::kr::dl::semantics::RelationStorage, 4)
