#ifndef RUNIR_KR_PS_REPOSITORY_HPP_
#define RUNIR_KR_PS_REPOSITORY_HPP_

#include "runir/kr/ps/canonicalization.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <tyr/common/type_list.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/symbol_repository.hpp>
#include <utility>

namespace runir::kr::ps
{

template<FamilyTag Family, typename RepositoryTypes, typename DlRepositoryPtr>
class BasicRepository : public std::enable_shared_from_this<BasicRepository<Family, RepositoryTypes, DlRepositoryPtr>>
{
    template<FamilyTag, typename, typename>
    friend class BasicRepositoryFactory;

private:
    using Self = BasicRepository<Family, RepositoryTypes, DlRepositoryPtr>;

    tyr::ApplyTypeListT<tyr::formalism::SymbolRepository, RepositoryTypes> m_symbol_repository;
    DlRepositoryPtr m_dl_repository;
    size_t m_index;

    BasicRepository(size_t index, DlRepositoryPtr dl_repository) : m_symbol_repository(nullptr), m_dl_repository(std::move(dl_repository)), m_index(index)
    {
        assert(m_dl_repository);
        clear();
    }

public:
    BasicRepository(const BasicRepository&) = delete;
    BasicRepository& operator=(const BasicRepository&) = delete;
    BasicRepository(BasicRepository&&) = delete;
    BasicRepository& operator=(BasicRepository&&) = delete;

    const auto& get_index() const noexcept { return m_index; }
    std::shared_ptr<const Self> get_shared_ptr() const noexcept
    {
        try
        {
            return this->shared_from_this();
        }
        catch (const std::bad_weak_ptr&)
        {
            return {};
        }
    }
    std::shared_ptr<Self> get_shared_ptr() noexcept
    {
        try
        {
            return this->shared_from_this();
        }
        catch (const std::bad_weak_ptr&)
        {
            return {};
        }
    }
    auto& get_dl_repository() noexcept { return *m_dl_repository; }
    const auto& get_dl_repository() const noexcept { return *m_dl_repository; }
    const auto& get_dl_repository_ptr() const noexcept { return m_dl_repository; }

    void clear() noexcept { m_symbol_repository.clear(); }

    template<typename T>
    std::optional<tyr::View<tyr::Index<T>, BasicRepository>> find(const tyr::Data<T>& data) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local<T>(data))
            return tyr::View<tyr::Index<T>, BasicRepository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<tyr::View<tyr::Index<T>, BasicRepository>, bool> get_or_create(tyr::Data<T>& data)
    {
        const auto [index, success] = m_symbol_repository.template get_or_create_local<T>(data);
        return { tyr::View<tyr::Index<T>, BasicRepository>(index, *this), success };
    }

    template<typename T>
    const tyr::Data<T>& operator[](tyr::Index<T> index) const noexcept
    {
        return m_symbol_repository.template at_local<T>(index);
    }

    template<typename T>
    size_t size() const noexcept
    {
        return m_symbol_repository.template local_size<T>();
    }

    template<typename T>
    const BasicRepository& get_canonical_context(tyr::Index<T>) const noexcept
    {
        return *this;
    }
};

template<FamilyTag Family, typename RepositoryTypes, typename DlRepositoryPtr>
class BasicRepositoryFactory
{
private:
    size_t m_next_index = 0;

public:
    using Repository = BasicRepository<Family, RepositoryTypes, DlRepositoryPtr>;
    std::shared_ptr<Repository> create(DlRepositoryPtr dl_repository)
    {
        return std::shared_ptr<Repository>(new Repository(m_next_index++, std::move(dl_repository)));
    }
};

template<FamilyTag Family, typename RepositoryTypes, typename DlRepositoryPtr>
inline const BasicRepository<Family, RepositoryTypes, DlRepositoryPtr>&
get_repository(const BasicRepository<Family, RepositoryTypes, DlRepositoryPtr>& repository) noexcept
{
    return repository;
}

template<FamilyTag Family, typename RepositoryTypes, typename DlRepositoryPtr>
inline BasicRepository<Family, RepositoryTypes, DlRepositoryPtr>& get_repository(BasicRepository<Family, RepositoryTypes, DlRepositoryPtr>& repository) noexcept
{
    return repository;
}

}  // namespace runir::kr::ps

#endif
