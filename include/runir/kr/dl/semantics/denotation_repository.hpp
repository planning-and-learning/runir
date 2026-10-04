#ifndef RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_
#define RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/canonicalization.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/denotation_index.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <tyr/formalism/planning/declarations.hpp>
#include <utility>
#include <yggdrasil/containers/raw_vector_set.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/database/relation_repository.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::dl::semantics
{

class DenotationRepositoryFactory
{
    friend class DenotationRepository;

private:
    // Copies share identity sequences, including factories retained by repositories.
    std::shared_ptr<size_t> m_next_index;
    ygg::database::RelationRepositoryFactory<> m_relation_factory;

public:
    DenotationRepositoryFactory() : m_next_index(std::make_shared<size_t>(0)) {}

    DenotationRepository create(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository);
    DenotationRepositoryPtr create_shared(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository);
};

class DenotationRepository
{
    friend class DenotationRepositoryFactory;

public:
    using SymbolRepository = ygg::formalism::
        SymbolRepository<Denotation<BooleanTag>, Denotation<NumericalTag>, Denotation<ConceptTag>, Denotation<RoleTag>, RegisterValues, CallArguments>;
    using SymbolTypes = SymbolRepository::SymbolTypes;
    using VectorRepository = ygg::RawVectorSet<ygg::uint_t, ygg::uint_t>;

private:
    SymbolRepository m_symbol_repository;
    VectorRepository m_vector_repository;
    ygg::database::RelationRepository<> m_relation_repository;
    std::shared_ptr<const tyr::formalism::planning::Repository> m_formalism_repository;
    DenotationRepositoryFactory m_factory;
    size_t m_index;

    DenotationRepository(size_t index, DenotationRepositoryFactory factory, std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository) :
        m_symbol_repository(nullptr),
        m_vector_repository(),
        m_relation_repository(factory.m_relation_factory.create()),
        m_formalism_repository(std::move(formalism_repository)),
        m_factory(std::move(factory)),
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
    auto get_factory() const noexcept { return m_factory; }
    const auto& get_formalism_repository() const noexcept
    {
        assert(m_formalism_repository);
        return *m_formalism_repository;
    }
    const auto& get_formalism_repository_ptr() const noexcept { return m_formalism_repository; }

    void clear() noexcept
    {
        m_symbol_repository.clear();
        m_vector_repository.clear();
        m_relation_repository.clear();
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
    std::optional<ygg::View<ygg::Index<T>, DenotationRepository>> find(const ygg::Data<T>& data) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local<T>(data))
            return ygg::View<ygg::Index<T>, DenotationRepository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
    std::pair<ygg::View<ygg::Index<T>, DenotationRepository>, bool> get_or_create(ygg::Data<T>& data)
    {
        const auto [index, created] = m_symbol_repository.template get_or_create_local<T>(data);
        return { ygg::View<ygg::Index<T>, DenotationRepository>(index, *this), created };
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const noexcept
    {
        assert(m_symbol_repository.template is_local<T>(index));
        return m_symbol_repository.template at_local<T>(index);
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
    size_t size() const noexcept
    {
        return m_symbol_repository.template local_size<T>();
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
    const DenotationRepository& get_canonical_context(ygg::Index<T>) const noexcept
    {
        return *this;
    }

    const auto& get_vector_repository() const noexcept { return m_vector_repository; }
    auto& get_vector_repository() noexcept { return m_vector_repository; }
    const auto& get_relation_repository() const noexcept { return m_relation_repository; }
    auto& get_relation_repository() noexcept { return m_relation_repository; }
};

inline DenotationRepository DenotationRepositoryFactory::create(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository)
{
    return DenotationRepository((*m_next_index)++, *this, std::move(formalism_repository));
}

inline DenotationRepositoryPtr DenotationRepositoryFactory::create_shared(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository)
{
    return DenotationRepositoryPtr(new DenotationRepository((*m_next_index)++, *this, std::move(formalism_repository)));
}

inline const DenotationRepository& get_denotation_repository(const DenotationRepository& repository) noexcept { return repository; }

inline const DenotationRepository::VectorRepository& get_denotation_vector_repository(const DenotationRepository& repository) noexcept
{
    return repository.get_vector_repository();
}

inline DenotationRepository::VectorRepository& get_denotation_vector_repository(DenotationRepository& repository) noexcept
{
    return repository.get_vector_repository();
}

template<typename T>
    requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
void prepare_for_interning(DenotationRepository&, ygg::Data<T>& data)
{
    canonicalize(data);
}

using ygg::formalism::get_or_create;

}

#endif
