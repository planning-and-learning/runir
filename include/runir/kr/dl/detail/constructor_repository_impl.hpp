#ifndef RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_IMPL_HPP_
#define RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_IMPL_HPP_

#include "runir/kr/dl/detail/constructor_repository.hpp"

#include <memory>
#include <utility>

namespace runir::kr::dl::detail
{

template<typename RepositoryTypes>
std::shared_ptr<ConstructorRepository<RepositoryTypes>>
ConstructorRepositoryFactory<RepositoryTypes>::create(std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository)
{
    return std::shared_ptr<ConstructorRepository<RepositoryTypes>>(new ConstructorRepository<RepositoryTypes>(m_next_index++, std::move(planning_repository)));
}

}  // namespace runir::kr::dl::detail

#endif
