#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"

#include <cassert>
#include <memory>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <utility>
#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::kr::dl::semantics
{

DenotationRepository::DenotationRepository(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository) :
    m_formalism_repository(std::move(formalism_repository)),
    m_index(ygg::next_layer())
{
    assert(m_formalism_repository);
    this->reset();
}

DenotationRepository::~DenotationRepository() = default;

}  // namespace runir::kr::dl::semantics

RUNIR_KR_DL_SEMANTICS_INTERNS()

// The symbols and builders of the library's repositories, which its other sources only declare.
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::dl::semantics::DenotationRepository, 7)
YGG_FORMALISM_BUILDER(, runir::kr::dl::semantics::RecordStorage, 7)
YGG_FORMALISM_BUILDER(, runir::kr::dl::semantics::RelationStorage, 4)
