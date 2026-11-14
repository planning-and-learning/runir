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
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::kr::ps
{

template<FamilyTag Family, typename RepositoryTypes>
class BasicRepository : public ygg::formalism::SymbolContext<BasicRepository<Family, RepositoryTypes>, RepositoryTypes>
{
    using Base = ygg::formalism::SymbolContext<BasicRepository<Family, RepositoryTypes>, RepositoryTypes>;

public:
    using DlRepositoryPtr = runir::kr::dl::ConstructorRepositoryPtr;

private:
    DlRepositoryPtr m_dl_repository;
    size_t m_index;

public:
    /// Over the DL repository whose records its rules refer to. Numbered by ygg::next_layer(), the process-wide counter,
    /// so that no two contexts share a number.
    explicit BasicRepository(DlRepositoryPtr dl_repository) : m_dl_repository(std::move(dl_repository)), m_index(ygg::next_layer())
    {
        assert(m_dl_repository);
        this->clear();
    }
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

    using Base::view;
    /// A DL symbol (a register, a constructor) as its view in the DL repository, which holds it: so that urd resolves
    /// every member of a rule through this context (urd's resolves contract).
    template<typename T>
        requires ygg::formalism::SupportsSymbol<runir::kr::dl::ConstructorRepository, T>
    [[nodiscard]] urd::index_view<T, runir::kr::dl::ConstructorRepository> view(urd::index<T> index) const
    { return get_dl_repository().view(index); }
};

template<FamilyTag Family, typename RepositoryTypes>
inline const BasicRepository<Family, RepositoryTypes>& get_repository(const BasicRepository<Family, RepositoryTypes>& repository URD_LIFETIMEBOUND) noexcept
{ return repository; }

template<FamilyTag Family, typename RepositoryTypes>
inline BasicRepository<Family, RepositoryTypes>& get_repository(BasicRepository<Family, RepositoryTypes>& repository) noexcept
{ return repository; }

}  // namespace runir::kr::ps


#endif
