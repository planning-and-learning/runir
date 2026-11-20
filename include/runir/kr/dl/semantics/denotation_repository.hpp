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
#include <yggdrasil/formalism/nested.hpp>
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::kr::dl::semantics
{

/// Denotations, over the planning repository whose objects they range over (nested, read only).
class DenotationRepository :
    public ygg::formalism::SymbolContext<DenotationRepository, DenotationRecordTypes>,
    public ygg::formalism::Nests<DenotationRepository, ygg::formalism::Nested<const tyr::formalism::planning::Repository>>
{
    using Base = ygg::formalism::SymbolContext<DenotationRepository, DenotationRecordTypes>;
    using NestsBase = ygg::formalism::Nests<DenotationRepository, ygg::formalism::Nested<const tyr::formalism::planning::Repository>>;

    ygg::FreeRelationRepository m_relation_repository;
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

    /// Clears the denotations together with the relation repository they refer to.
    void reset() noexcept
    {
        Base::clear();
        m_relation_repository.clear();
    }

    using Base::find;
    using Base::insert;
    using Base::view;
    using NestsBase::find;
    using NestsBase::get;
    using NestsBase::get_ptr;
    using NestsBase::insert;
    using NestsBase::repository_for;
    using NestsBase::view;

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
