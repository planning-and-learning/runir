#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <cassert>
#include <memory>
#include <utility>
#include <yggdrasil/database/syntax/query_repository.hpp>
#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::RegisterValues>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::CallArguments>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Columns<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int> >>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Relation<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int> >>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, void>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryInputTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryEmptyTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryJoinTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryProjectTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryRenameTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QuerySelectEqualTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QuerySelectValueTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryUnionTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryDifferenceTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryDistanceTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryGenericJoinTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QuerySemijoinTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryAntijoinTag>>;

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
    this->clear();
}

DenotationRepository::~DenotationRepository() = default;

DenotationRepository DenotationRepositoryFactory::create(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository)
{
    return DenotationRepository((*m_next_index)++, *this, std::move(formalism_repository));
}

DenotationRepositoryPtr DenotationRepositoryFactory::create_shared(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository)
{
    return DenotationRepositoryPtr(new DenotationRepository((*m_next_index)++, *this, std::move(formalism_repository)));
}

}  // namespace runir::kr::dl::semantics
