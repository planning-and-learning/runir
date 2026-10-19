#ifndef RUNIR_KR_PS_DETAIL_REPOSITORY_IMPL_HPP_
#define RUNIR_KR_PS_DETAIL_REPOSITORY_IMPL_HPP_

#include "runir/kr/ps/repository.hpp"

#include <memory>
#include <utility>

namespace runir::kr::ps
{

template<FamilyTag Family, typename RepositoryTypes>
std::shared_ptr<BasicRepository<Family, RepositoryTypes>> BasicRepositoryFactory<Family, RepositoryTypes>::create(DlRepositoryPtr dl_repository)
{
    return std::shared_ptr<Repository>(new Repository(m_next_index++, std::move(dl_repository)));
}

}  // namespace runir::kr::ps

#endif
