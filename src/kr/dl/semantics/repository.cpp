#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <cassert>
#include <memory>
#include <utility>
#include <yggdrasil/database/syntax/query_repository.hpp>

namespace runir::kr::dl::semantics
{

DenotationRepository::DenotationRepository(size_t index,
                                           DenotationRepositoryFactory factory,
                                           std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository) :
    m_vector_repository(),
    m_relation_repository(factory.m_relation_factory.create()),
    m_formalism_repository(std::move(formalism_repository)),
    m_factory(std::move(factory)),
    m_index(index)
{
    assert(m_formalism_repository);
    this->reset();
}

DenotationRepository::~DenotationRepository() = default;

DenotationRepository DenotationRepositoryFactory::create(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository)
{ return DenotationRepository(m_indices.next(), *this, std::move(formalism_repository)); }

DenotationRepositoryPtr DenotationRepositoryFactory::create_shared(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository)
{ return DenotationRepositoryPtr(new DenotationRepository(m_indices.next(), *this, std::move(formalism_repository))); }

}  // namespace runir::kr::dl::semantics
