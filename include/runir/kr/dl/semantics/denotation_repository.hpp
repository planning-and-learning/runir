#ifndef RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_
#define RUNIR_SEMANTICS_DENOTATION_REPOSITORY_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "tyr/formalism/repository.hpp"

#include <boost/mp11.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <runir/kr/database/semantics/relation_view.hpp>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <urd/config.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/pools/pools.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/index_sequence.hpp>
#include <yggdrasil/formalism/declarations.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/keyed.hpp>
#include <yggdrasil/formalism/nested.hpp>
#include <yggdrasil/formalism/part_instantiations.hpp>

namespace runir::kr::dl::semantics
{

/// Denotations, over the planning repository whose objects they range over (nested, read only). A keyed repository
/// (ygg::formalism::KeyedRepository) of its own parts, as tyr's state repository is: its symbols (Boolean and numerical
/// denotations, register values, call arguments), and a pool for its concept denotations and one for its role
/// denotations (DenotationPool: each packed into one row of words, its key the row's number). Its records are read,
/// viewed, found and inserted, and its builders interned, by the keyed members (operator[], at, view, find, insert).
class DenotationRepository :
    public ygg::formalism::KeyedRepository<DenotationRepository>,
    public ygg::formalism::Nests<DenotationRepository, ygg::formalism::Nested<const tyr::formalism::Repository>>
{
    using NestsBase = ygg::formalism::Nests<DenotationRepository, ygg::formalism::Nested<const tyr::formalism::Repository>>;

    /// Its stores: defined in repository.cpp, so that no other source completes one.
    struct Impl;

    std::unique_ptr<Impl> m_impl;
    ygg::FreeRelationRepository m_relation_repository;
    size_t m_index;

public:
    /// The kinds its symbol part holds: every kind but its concept and role denotations, which its pools hold.
    using SymbolKinds = boost::mp11::mp_list<Denotation<BooleanTag>, Denotation<NumericalTag>, RegisterValues, CallArguments>;
    using Symbols = ygg::formalism::SymbolStores<false, Denotation<BooleanTag>, Denotation<NumericalTag>, RegisterValues, CallArguments>;

    /// The kinds of record it holds.
    using SymbolTypes = DenotationRecordTypes;
    /// The keys it resolves itself: its records'.
    using KeyTypes = boost::mp11::mp_transform<urd::key, DenotationRecordTypes>;

    /// The part of its own that holds the records of the keys Keys in Store, read (Writable false) or written into.
    template<bool Writable, typename Store, typename Keys>
    using Part = ygg::formalism::
        HeldPart<std::conditional_t<Writable, DenotationRepository, const DenotationRepository>, std::conditional_t<Writable, Store, const Store>, Keys>;

    /// Its own parts (ygg::formalism::key_parts): its symbols, and the pools of its concept and role denotations.
    template<bool Writable>
    using OwnParts = std::tuple<Part<Writable, Symbols, boost::mp11::mp_transform<urd::key, SymbolKinds>>,
                                Part<Writable, DenotationPool<ConceptTag>, boost::mp11::mp_list<urd::key<Denotation<ConceptTag>>>>,
                                Part<Writable, DenotationPool<RoleTag>, boost::mp11::mp_list<urd::key<Denotation<RoleTag>>>>>;

    /// Over the planning repository whose objects its denotations range over. Numbered by ygg::next_layer(), the
    /// process-wide counter, so that no two contexts share a number. The constructor and destructor are defined in the
    /// library (template rule 7): inline ones would construct and destroy its stores and relation pools in each caller.
    explicit DenotationRepository(std::shared_ptr<const tyr::formalism::Repository> formalism_repository);
    ~DenotationRepository();
    DenotationRepository(const DenotationRepository&) = delete;
    DenotationRepository& operator=(const DenotationRepository&) = delete;
    DenotationRepository(DenotationRepository&&) = delete;
    DenotationRepository& operator=(DenotationRepository&&) = delete;

    /// No layer below it.
    [[nodiscard]] const DenotationRepository* parent() const noexcept { return nullptr; }

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }

    /// Clears the denotations together with the relation repository they refer to.
    void reset() noexcept;

    using KeyedRepository::find;
    using KeyedRepository::insert;
    using KeyedRepository::view;
    using NestsBase::get;
    using NestsBase::get_ptr;

    [[nodiscard]] const auto& get_relation_repository() const noexcept { return m_relation_repository; }

    auto& get_relation_repository() noexcept { return m_relation_repository; }

private:
    friend class ygg::formalism::KeyedRepository<DenotationRepository>;

    /// Its own parts as read; written into, likewise: its Impl's stores (repository.cpp).
    [[nodiscard]] OwnParts<false> own_parts() const;
    [[nodiscard]] OwnParts<true> own_parts();
};

}

// Its parts' reads, finds and inserts instantiated once, where the library instantiates the repository (brokkr's
// include rule I7): a file that reads records through it completes none of its stores.
YGG_FORMALISM_PART(extern, runir::kr::dl::semantics::DenotationRepository, 0, 4)
YGG_FORMALISM_PART(extern, runir::kr::dl::semantics::DenotationRepository, 1, 1)
YGG_FORMALISM_PART(extern, runir::kr::dl::semantics::DenotationRepository, 2, 1)

#endif
