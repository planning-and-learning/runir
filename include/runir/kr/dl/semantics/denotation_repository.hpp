#ifndef RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_
#define RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_

#include "runir/config.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/canonicalization.hpp"
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
    mutable ygg::Data<Denotation<BooleanTag>> m_boolean_data;
    mutable ygg::Data<Denotation<NumericalTag>> m_numerical_data;
    mutable ygg::Data<Denotation<ConceptTag>> m_concept_data;
    mutable ygg::Data<Denotation<RoleTag>> m_role_data;
    size_t m_index;

    template<CategoryTag Category>
    std::optional<ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>> find_with_hash(const ygg::Data<Denotation<Category>>& data,
                                                                                                    size_t hash) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<Denotation<Category>>(data, hash))
            return ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>(*index, *this);
        return std::nullopt;
    }

    template<CategoryTag Category>
    std::pair<ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>, bool> get_or_create_with_hash(ygg::Data<Denotation<Category>>& data,
                                                                                                               size_t hash)
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<Denotation<Category>>(data, hash))
            return { ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>(*index, *this), false };

        const auto [index, success] = m_symbol_repository.template get_or_create_local_with_hash<Denotation<Category>>(data, hash);
        return { ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>(index, *this), success };
    }

    const ygg::Data<Denotation<BooleanTag>>& make_data(const ygg::Builder<Denotation<BooleanTag>>& builder) const noexcept
    {
        m_boolean_data = ygg::Data<Denotation<BooleanTag>>(builder.value);
        return m_boolean_data;
    }

    ygg::Data<Denotation<BooleanTag>>& make_data(const ygg::Builder<Denotation<BooleanTag>>& builder) noexcept
    {
        m_boolean_data = ygg::Data<Denotation<BooleanTag>>(builder.value);
        return m_boolean_data;
    }

    const ygg::Data<Denotation<NumericalTag>>& make_data(const ygg::Builder<Denotation<NumericalTag>>& builder) const noexcept
    {
        m_numerical_data = ygg::Data<Denotation<NumericalTag>>(builder.value);
        return m_numerical_data;
    }

    ygg::Data<Denotation<NumericalTag>>& make_data(const ygg::Builder<Denotation<NumericalTag>>& builder) noexcept
    {
        m_numerical_data = ygg::Data<Denotation<NumericalTag>>(builder.value);
        return m_numerical_data;
    }

    std::optional<std::reference_wrapper<const ygg::Data<Denotation<ConceptTag>>>> make_data(const ygg::Builder<Denotation<ConceptTag>>& builder) const noexcept
    {
        if (auto vec_index = m_vector_repository.find(builder.blocks))
        {
            m_concept_data = ygg::Data<Denotation<ConceptTag>>(builder.num_objects, *vec_index);
            return m_concept_data;
        }

        return std::nullopt;
    }

    ygg::Data<Denotation<ConceptTag>>& make_data(const ygg::Builder<Denotation<ConceptTag>>& builder)
    {
        m_concept_data = ygg::Data<Denotation<ConceptTag>>(builder.num_objects, m_vector_repository.insert(builder.blocks));
        return m_concept_data;
    }

    std::optional<std::reference_wrapper<const ygg::Data<Denotation<RoleTag>>>> make_data(const ygg::Builder<Denotation<RoleTag>>& builder) const noexcept
    {
        if (auto vec_index = m_vector_repository.find(builder.blocks))
        {
            m_role_data = ygg::Data<Denotation<RoleTag>>(builder.num_objects, *vec_index);
            return m_role_data;
        }

        return std::nullopt;
    }

    ygg::Data<Denotation<RoleTag>>& make_data(const ygg::Builder<Denotation<RoleTag>>& builder)
    {
        m_role_data = ygg::Data<Denotation<RoleTag>>(builder.num_objects, m_vector_repository.insert(builder.blocks));
        return m_role_data;
    }

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
        m_boolean_data.clear();
        m_numerical_data.clear();
        m_concept_data.clear();
        m_role_data.clear();
    }

    template<CategoryTag Category>
    std::optional<ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>> find(const ygg::Data<Denotation<Category>>& data) const noexcept
    {
        return find_with_hash<Category>(data, SymbolRepository::template hash<Denotation<Category>>(data));
    }

    std::optional<ygg::View<ygg::Index<Denotation<ConceptTag>>, DenotationRepository>> find(const ygg::Builder<Denotation<ConceptTag>>& builder) const noexcept
    {
        if (auto data = make_data(builder))
            return find(data->get());

        return std::nullopt;
    }

    std::optional<ygg::View<ygg::Index<Denotation<RoleTag>>, DenotationRepository>> find(const ygg::Builder<Denotation<RoleTag>>& builder) const noexcept
    {
        if (auto data = make_data(builder))
            return find(data->get());

        return std::nullopt;
    }

    template<CategoryTag Category>
    std::pair<ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>, bool> get_or_create(ygg::Data<Denotation<Category>>& data)
    {
        return get_or_create_with_hash<Category>(data, SymbolRepository::template hash<Denotation<Category>>(data));
    }

    std::pair<ygg::View<ygg::Index<Denotation<BooleanTag>>, DenotationRepository>, bool> get_or_create(ygg::Builder<Denotation<BooleanTag>>& builder)
    {
        auto& data = make_data(builder);
        auto [view, created] = get_or_create(data);
        builder.index = view.get_index();
        return { view, created };
    }

    std::pair<ygg::View<ygg::Index<Denotation<NumericalTag>>, DenotationRepository>, bool> get_or_create(ygg::Builder<Denotation<NumericalTag>>& builder)
    {
        auto& data = make_data(builder);
        auto [view, created] = get_or_create(data);
        builder.index = view.get_index();
        return { view, created };
    }

    std::pair<ygg::View<ygg::Index<Denotation<ConceptTag>>, DenotationRepository>, bool> get_or_create(ygg::Builder<Denotation<ConceptTag>>& builder)
    {
        auto& data = make_data(builder);
        auto [view, created] = get_or_create(data);
        builder.index = view.get_index();
        return { view, created };
    }

    std::pair<ygg::View<ygg::Index<Denotation<RoleTag>>, DenotationRepository>, bool> get_or_create(ygg::Builder<Denotation<RoleTag>>& builder)
    {
        auto& data = make_data(builder);
        auto [view, created] = get_or_create(data);
        builder.index = view.get_index();
        return { view, created };
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

template<CategoryTag Category>
using DenotationView = ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>;

using ConceptDenotationView = DenotationView<ConceptTag>;
using DenotationRepositoryPtr = std::shared_ptr<DenotationRepository>;

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
