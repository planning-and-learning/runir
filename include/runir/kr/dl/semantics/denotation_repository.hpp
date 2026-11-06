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
#include <urd/repr/pools/pools.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <yggdrasil/core/config.hpp>
#include <urd/config.hpp>
#include <urd/elements/flat.hpp>
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
    ygg::database::RelationRepositoryFactory m_relation_factory;

public:
    DenotationRepositoryFactory() : m_next_index(std::make_shared<size_t>(0)) {}

    DenotationRepository create(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository);
    DenotationRepositoryPtr create_shared(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository);
};

class DenotationRepository : public ygg::formalism::SymbolRepositoryBase<DenotationRepository, DenotationRecordTypes>
{
    using Base = ygg::formalism::SymbolRepositoryBase<DenotationRepository, DenotationRecordTypes>;

    friend class DenotationRepositoryFactory;

    /// The denotations' blocks, interned in a urd pool (stored flat): a handle's value is a vec_index.
    using VectorRepository = urd::Store<urd::pool<urd::Store<urd::sequence<ygg::uint_t>, urd::frozen>>>;

    VectorRepository m_vector_repository;
    ygg::database::RelationRepository m_relation_repository;
    std::shared_ptr<const tyr::formalism::planning::Repository> m_formalism_repository;
    DenotationRepositoryFactory m_factory;
    size_t m_index;

    // Constructor, destructor and factory functions are defined in the library (template rule 7): inline ones
    // would construct and destroy every symbol pool in each caller, and those pools are instantiated there.
    DenotationRepository(size_t index, DenotationRepositoryFactory factory, std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository);

public:
    ~DenotationRepository();
    DenotationRepository(const DenotationRepository&) = delete;
    DenotationRepository& operator=(const DenotationRepository&) = delete;
    DenotationRepository(DenotationRepository&&) = delete;
    DenotationRepository& operator=(DenotationRepository&&) = delete;

    [[nodiscard]] const auto& get_index() const noexcept { return m_index; }
    /// The repository's number, as urd::view asks of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }
    [[nodiscard]] auto get_factory() const noexcept { return m_factory; }
    [[nodiscard]] const tyr::formalism::planning::Repository& get_formalism_repository() const noexcept
    {
        assert(m_formalism_repository);
        // Dereference the raw pointer: operator* on the shared_ptr would complete the repository type for its lookup.
        return *m_formalism_repository.get();
    }
    [[nodiscard]] const auto& get_formalism_repository_ptr() const noexcept { return m_formalism_repository; }

    /// Clears the denotations together with the vector and relation repositories they refer to.
    void reset() noexcept
    {
        Base::clear();
        m_vector_repository.clear();
        m_relation_repository.clear();
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
    const urd::data<T>& operator[](urd::index<T> index) const noexcept
    {
        assert(this->template is_local<T>(index));
        return this->template local<T>(index);
    }

    /// An index as its view here (urd's resolves contract).
    template<typename T>
        requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
    [[nodiscard]] urd::view<T, DenotationRepository> view(urd::index<T> index) const noexcept
    { return { index, *this }; }

    /// The blocks of vec_index `index`.
    [[nodiscard]] std::span<const ygg::uint_t> vector(ygg::uint_t index) const { return m_vector_repository.view(VectorRepository::handle_type(index)); }
    /// How many distinct block vectors are stored.
    [[nodiscard]] std::size_t vector_count() const noexcept { return m_vector_repository.size(); }
    /// The vec_index of these blocks, stored where they are new.
    ygg::uint_t insert_vector(std::span<const ygg::uint_t> blocks) { return m_vector_repository.insert(blocks).first.value; }
    [[nodiscard]] const auto& get_relation_repository() const noexcept { return m_relation_repository; }
    auto& get_relation_repository() noexcept { return m_relation_repository; }
};

/// A repository owns the records viewed through it.
inline const DenotationRepository& get_repository(const DenotationRepository& repository URD_LIFETIMEBOUND) noexcept { return repository; }

template<typename T>
    requires ygg::formalism::SupportsSymbol<DenotationRepository, T>
void prepare_for_insert(DenotationRepository&, urd::data<T>& data)
{ canonicalize(data); }

using ygg::formalism::insert;
}

#endif
