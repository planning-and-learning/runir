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
#include <tyr/datalog/lifted/database/semantics/relation_view.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <urd/config.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/pools/pools.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/index_sequence.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::kr::dl::semantics
{

class DenotationRepository : public ygg::formalism::SymbolContext<DenotationRepository, DenotationRecordTypes>
{
    using Base = ygg::formalism::SymbolContext<DenotationRepository, DenotationRecordTypes>;

    /// The denotations' blocks, interned in a urd pool (stored flat): a handle is a vec_index.
    using VectorRepository = urd::Store<urd::pool<DenotationBlocks>>;

    VectorRepository m_vector_repository;
    ygg::FreeRelationRepository m_relation_repository;
    std::shared_ptr<const tyr::formalism::planning::Repository> m_formalism_repository;
    size_t m_index;

public:
    /// Over the planning repository whose objects its denotations range over. Numbered by ygg::next_layer(), the
    /// process-wide counter, so that no two contexts share a number. The constructor and destructor are defined in the
    /// library (template rule 7): inline ones would construct and destroy the vector and relation pools in each caller.
    explicit DenotationRepository(std::shared_ptr<const tyr::formalism::planning::Repository> formalism_repository);
    ~DenotationRepository();
    DenotationRepository(const DenotationRepository&) = delete;
    DenotationRepository& operator=(const DenotationRepository&) = delete;
    DenotationRepository(DenotationRepository&&) = delete;
    DenotationRepository& operator=(DenotationRepository&&) = delete;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }
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

    [[nodiscard]] const auto& get_vector_repository() const noexcept { return m_vector_repository; }
    auto& get_vector_repository() noexcept { return m_vector_repository; }
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
