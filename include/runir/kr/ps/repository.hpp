#ifndef RUNIR_KR_PS_REPOSITORY_HPP_
#define RUNIR_KR_PS_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/canonicalization.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <boost/mp11.hpp>
#include <cassert>
#include <memory>
#include <urd/config.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::ps
{

template<FamilyTag Family, typename RepositoryTypes>
class BasicRepository : public ygg::formalism::SymbolRepositoryBase<BasicRepository<Family, RepositoryTypes>, RepositoryTypes>
{
    using Base = ygg::formalism::SymbolRepositoryBase<BasicRepository<Family, RepositoryTypes>, RepositoryTypes>;

public:
    using DlRepositoryPtr = runir::kr::dl::ConstructorRepositoryPtr;

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
    ~BasicRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }
    // Dereference the raw pointer: operator* on the shared_ptr would complete the DL repository type for its lookup.
    auto& get_dl_repository() noexcept { return *m_dl_repository.get(); }
    [[nodiscard]] const auto& get_dl_repository() const noexcept { return *m_dl_repository.get(); }
    [[nodiscard]] const auto& get_dl_repository_ptr() const noexcept { return m_dl_repository; }
};

template<FamilyTag Family, typename RepositoryTypes>
class BasicRepositoryFactory
{
private:
    size_t m_next_index = 0;

public:
    using Repository = BasicRepository<Family, RepositoryTypes>;
    using DlRepositoryPtr = typename Repository::DlRepositoryPtr;
    /// Defined in detail/repository_impl.hpp: an in-class definition would construct and destroy every symbol pool
    /// in each caller (template rule 7).
    std::shared_ptr<Repository> create(DlRepositoryPtr dl_repository);
};

template<FamilyTag Family, typename RepositoryTypes>
inline const BasicRepository<Family, RepositoryTypes>& get_repository(const BasicRepository<Family, RepositoryTypes>& repository URD_LIFETIMEBOUND) noexcept
{ return repository; }

template<FamilyTag Family, typename RepositoryTypes>
inline BasicRepository<Family, RepositoryTypes>& get_repository(BasicRepository<Family, RepositoryTypes>& repository) noexcept
{ return repository; }

}  // namespace runir::kr::ps

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/ps/detail/repository_impl.hpp"
#endif

#endif
