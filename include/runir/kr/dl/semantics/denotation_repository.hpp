#ifndef RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_
#define RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/canonicalization.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"

#include <cassert>
#include <memory>
#include <tyr/formalism/planning/declarations.hpp>
#include <utility>
#include <yggdrasil/containers/raw_vector_set.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/database/semantics/relation_repository.hpp>
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
    ygg::database::RelationRepositoryFactory<ObjectValues> m_relation_factory;

public:
    DenotationRepositoryFactory() : m_next_index(std::make_shared<size_t>(0)) {}

    DenotationRepository create(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository);
    DenotationRepositoryPtr create_shared(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository);
};

class DenotationRepository : public ygg::formalism::SymbolRepositoryBase<DenotationRepository, DenotationRecordTypes>
{
    using Base = ygg::formalism::SymbolRepositoryBase<DenotationRepository, DenotationRecordTypes>;

    friend class DenotationRepositoryFactory;

public:
    using VectorRepository = ygg::RawVectorSet<ygg::uint_t, ygg::uint_t>;

private:
    VectorRepository m_vector_repository;
    ygg::database::RelationRepository<ObjectValues> m_relation_repository;
    std::shared_ptr<const tyr::formalism::planning::Repository> m_formalism_repository;
    DenotationRepositoryFactory m_factory;
    size_t m_index;

    DenotationRepository(size_t index, DenotationRepositoryFactory factory, std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository) :
        m_vector_repository(),
        m_relation_repository(factory.m_relation_factory.create()),
        m_formalism_repository(std::move(formalism_repository)),
        m_factory(std::move(factory)),
        m_index(index)
    {
        assert(m_formalism_repository);
        this->clear();
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
        Base::clear();
        m_vector_repository.clear();
        m_relation_repository.clear();
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const noexcept
    {
        assert(this->template is_local<T>(index));
        return this->template at_local<T>(index);
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

/// A repository owns the records viewed through it.
inline const DenotationRepository& get_repository(const DenotationRepository& repository) noexcept { return repository; }

template<typename T>
    requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
void prepare_for_insert(DenotationRepository&, ygg::Data<T>& data)
{
    canonicalize(data);
}

using ygg::formalism::insert;
}

#endif
