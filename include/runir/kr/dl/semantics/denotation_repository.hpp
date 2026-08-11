#ifndef RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_
#define RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_

#include "runir/kr/dl/semantics/canonicalization.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/denotation_index.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <tuple>
#include <tyr/formalism/planning/declarations.hpp>
#include <utility>
#include <yggdrasil/containers/raw_vector_set.hpp>
#include <yggdrasil/containers/unique_object_pool.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::dl::semantics
{

template<typename T>
class BasicBuilder
{
private:
    ygg::UniqueObjectPool<ygg::Builder<T>> m_pool;

public:
    [[nodiscard]] auto get_builder() { return m_pool.get_or_allocate(); }

    template<typename... Args>
    [[nodiscard]] auto get_builder(Args&&... args)
    {
        return m_pool.get_or_allocate(std::forward<Args>(args)...);
    }
};

class Builder
{
private:
    using DenotationBuilderStorage = std::tuple<BasicBuilder<Denotation<BooleanTag>>,
                                                BasicBuilder<Denotation<NumericalTag>>,
                                                BasicBuilder<Denotation<ConceptTag>>,
                                                BasicBuilder<Denotation<RoleTag>>>;
    using DenotationDataStorage = ygg::formalism::BuilderStorage<Denotation<BooleanTag>, Denotation<NumericalTag>, Denotation<ConceptTag>, Denotation<RoleTag>>;

    DenotationBuilderStorage m_builders;
    DenotationDataStorage m_data;

public:
    template<typename T>
    [[nodiscard]] auto get_builder()
    {
        return std::get<BasicBuilder<T>>(m_builders).get_builder();
    }

    template<typename T, typename... Args>
    [[nodiscard]] auto get_builder(Args&&... args)
    {
        return std::get<BasicBuilder<T>>(m_builders).get_builder(std::forward<Args>(args)...);
    }

    template<typename T>
    [[nodiscard]] auto get_data()
    {
        return m_data.template get_builder<T>();
    }
};

template<typename T>
[[nodiscard]] auto checkout(Builder& builder)
{
    auto data = builder.template get_data<T>();
    data->clear();
    return data;
}

inline ygg::Data<Denotation<BooleanTag>>& make_data(const ygg::Builder<Denotation<BooleanTag>>& builder, ygg::Data<Denotation<BooleanTag>>& data) noexcept
{
    data.value = builder.value;
    return data;
}

inline ygg::Data<Denotation<NumericalTag>>& make_data(const ygg::Builder<Denotation<NumericalTag>>& builder, ygg::Data<Denotation<NumericalTag>>& data) noexcept
{
    data.value = builder.value;
    return data;
}

inline ygg::Data<Denotation<ConceptTag>>& make_data(const ygg::Builder<Denotation<ConceptTag>>& builder, ygg::Data<Denotation<ConceptTag>>& data) noexcept
{
    data.num_objects = builder.num_objects;
    return data;
}

inline ygg::Data<Denotation<RoleTag>>& make_data(const ygg::Builder<Denotation<RoleTag>>& builder, ygg::Data<Denotation<RoleTag>>& data) noexcept
{
    data.num_objects = builder.num_objects;
    return data;
}

class DenotationRepository
{
    friend class DenotationRepositoryFactory;

public:
    using SymbolRepository = ygg::formalism::SymbolRepository<Denotation<BooleanTag>, Denotation<NumericalTag>, Denotation<ConceptTag>, Denotation<RoleTag>>;
    using VectorRepository = ygg::RawVectorSet<ygg::uint_t, ygg::uint_t>;

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

template<CategoryTag Category>
[[nodiscard]] auto get_or_create(DenotationRepository& repository, ygg::Data<Denotation<Category>>& data)
{
    canonicalize(data);
    return repository.get_or_create(data);
}

}

#endif
