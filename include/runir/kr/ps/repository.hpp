#ifndef RUNIR_KR_PS_REPOSITORY_HPP_
#define RUNIR_KR_PS_REPOSITORY_HPP_

#include "runir/kr/ps/canonicalization.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <cassert>
#include <memory>
#include <utility>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::ps
{

template<FamilyTag Family, typename RepositoryTypes>
class BasicRepository : public ygg::formalism::SymbolRepositoryBase<BasicRepository<Family, RepositoryTypes>, RepositoryTypes>
{
    using Base = ygg::formalism::SymbolRepositoryBase<BasicRepository<Family, RepositoryTypes>, RepositoryTypes>;

public:
    using DlRepositoryPtr = runir::kr::dl::ConstructorRepositoryPtrFor<typename PsFamilyTraits<Family>::DlFamily>;

private:
    template<FamilyTag, typename>
    friend class BasicRepositoryFactory;

    DlRepositoryPtr m_dl_repository;
    size_t m_index;

    BasicRepository(size_t index, DlRepositoryPtr dl_repository) : m_dl_repository(std::move(dl_repository)), m_index(index)
    {
        assert(m_dl_repository);
        this->clear();
    }

public:
    BasicRepository(const BasicRepository&) = delete;
    BasicRepository& operator=(const BasicRepository&) = delete;
    BasicRepository(BasicRepository&&) = delete;
    BasicRepository& operator=(BasicRepository&&) = delete;

    const auto& get_index() const noexcept { return m_index; }
    auto& get_dl_repository() noexcept { return *m_dl_repository; }
    const auto& get_dl_repository() const noexcept { return *m_dl_repository; }
    const auto& get_dl_repository_ptr() const noexcept { return m_dl_repository; }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<BasicRepository, T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const noexcept
    {
        return this->template at_local<T>(index);
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<BasicRepository, T>
    const BasicRepository& get_canonical_context(ygg::Index<T>) const noexcept
    {
        return *this;
    }
};

template<FamilyTag Family, typename RepositoryTypes>
class BasicRepositoryFactory
{
private:
    size_t m_next_index = 0;

public:
    using Repository = BasicRepository<Family, RepositoryTypes>;
    using DlRepositoryPtr = typename Repository::DlRepositoryPtr;
    std::shared_ptr<Repository> create(DlRepositoryPtr dl_repository)
    {
        return std::shared_ptr<Repository>(new Repository(m_next_index++, std::move(dl_repository)));
    }
};

template<FamilyTag Family, typename RepositoryTypes>
inline const BasicRepository<Family, RepositoryTypes>& get_repository(const BasicRepository<Family, RepositoryTypes>& repository) noexcept
{
    return repository;
}

template<FamilyTag Family, typename RepositoryTypes>
inline BasicRepository<Family, RepositoryTypes>& get_repository(BasicRepository<Family, RepositoryTypes>& repository) noexcept
{
    return repository;
}

}  // namespace runir::kr::ps

#endif
