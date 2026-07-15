#ifndef RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_
#define RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_

#include "runir/config.hpp"
#include "runir/kr/dl/semantics/canonicalization.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/denotation_index.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
#include <yggdrasil/containers/raw_vector_set.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::dl::semantics
{

class DenotationRepository
{
    friend class DenotationRepositoryFactory;

public:
    using SymbolRepository = ygg::formalism::SymbolRepository<Denotation<BooleanTag>, Denotation<NumericalTag>, Denotation<ConceptTag>, Denotation<RoleTag>>;
    using VectorRepository = ygg::RawVectorSet<uint_t, uint_t>;

private:
    SymbolRepository m_symbol_repository;
    VectorRepository m_vector_repository;
    std::shared_ptr<const tyr::formalism::planning::Repository> m_formalism_repository;
    size_t m_index;

    DenotationRepository(size_t index, std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository) :
        m_symbol_repository(nullptr),
        m_vector_repository(),
        m_formalism_repository(std::move(formalism_repository)),
        m_index(index)
    {
        assert(m_formalism_repository);
        clear();
    }

public:
    DenotationRepository(const DenotationRepository&) = delete;
    DenotationRepository& operator=(const DenotationRepository&) = delete;
    DenotationRepository(DenotationRepository&&) = delete;
    DenotationRepository& operator=(DenotationRepository&&) = delete;

    const auto& get_index() const noexcept { return m_index; }
    const auto& get_formalism_repository() const noexcept
    {
        assert(m_formalism_repository);
        return *m_formalism_repository;
    }

    void clear() noexcept
    {
        m_symbol_repository.clear();
        m_vector_repository.clear();
    }

    template<CategoryTag Category>
    std::optional<ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>> find(const ygg::Data<Denotation<Category>>& data) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local<Denotation<Category>>(data))
            return ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>(*index, *this);
        return std::nullopt;
    }

    template<CategoryTag Category>
    std::pair<ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>, bool> get_or_create(ygg::Data<Denotation<Category>>& data)
    {
        const auto [index, created] = m_symbol_repository.template get_or_create_local<Denotation<Category>>(data);
        return { ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>(index, *this), created };
    }

    template<CategoryTag Category>
    const ygg::Data<Denotation<Category>>& operator[](ygg::Index<Denotation<Category>> index) const noexcept
    {
        assert(m_symbol_repository.template is_local<Denotation<Category>>(index));
        return m_symbol_repository.template at_local<Denotation<Category>>(index);
    }

    template<CategoryTag Category>
    size_t size() const noexcept
    {
        return m_symbol_repository.template local_size<Denotation<Category>>();
    }

    template<CategoryTag Category>
    const DenotationRepository& get_canonical_context(ygg::Index<Denotation<Category>>) const noexcept
    {
        return *this;
    }

    const auto& get_vector_repository() const noexcept { return m_vector_repository; }
    auto& get_vector_repository() noexcept { return m_vector_repository; }
};

class DenotationRepositoryFactory
{
private:
    size_t m_next_index;

public:
    DenotationRepositoryFactory() : m_next_index(0) {}

    DenotationRepository create(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository)
    {
        return DenotationRepository(m_next_index++, std::move(formalism_repository));
    }

    DenotationRepositoryPtr create_shared(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository)
    {
        return DenotationRepositoryPtr(new DenotationRepository(m_next_index++, std::move(formalism_repository)));
    }
};

inline const DenotationRepository& get_denotation_repository(const DenotationRepository& repository) noexcept { return repository; }

inline const DenotationRepository::VectorRepository& get_denotation_vector_repository(const DenotationRepository& repository) noexcept
{
    return repository.get_vector_repository();
}

inline DenotationRepository::VectorRepository& get_denotation_vector_repository(DenotationRepository& repository) noexcept
{
    return repository.get_vector_repository();
}

}

#endif
