#ifndef RUNIR_KR_DL_CNF_GRAMMAR_GENERATION_STATE_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_GENERATION_STATE_HPP_

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/cnf_grammar/generate.hpp"
#include "runir/kr/dl/cnf_grammar/non_terminal_view.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <cstddef>
#include <optional>
#include <tuple>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/pools/pools.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <utility>
#include <yggdrasil/core/chrono.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::dl::cnf_grammar::detail
{

/// The constructors generated so far, by the nonterminal that derives them and their complexity, in the order
/// generated: a multimap per category.
class GeneratedSentences
{
private:
    template<runir::kr::dl::CategoryTag Category>
    using Sentences = urd::Store<urd::map<std::pair<urd::index<NonTerminal<Category>>, size_t>, urd::index<runir::kr::dl::Constructor<Category>>>, urd::multi>;

    std::tuple<Sentences<runir::kr::dl::ConceptTag>,
               Sentences<runir::kr::dl::RoleTag>,
               Sentences<runir::kr::dl::BooleanTag>,
               Sentences<runir::kr::dl::NumericalTag>>
        m_sentences;

public:
    GeneratedSentences();
    ~GeneratedSentences();

    GeneratedSentences(const GeneratedSentences&) = delete;
    GeneratedSentences& operator=(const GeneratedSentences&) = delete;
    GeneratedSentences(GeneratedSentences&&) = delete;
    GeneratedSentences& operator=(GeneratedSentences&&) = delete;

    template<runir::kr::dl::CategoryTag Category>
    void add(NonTerminalView<Category> nonterminal, size_t complexity, urd::index<runir::kr::dl::Constructor<Category>> constructor);

    /// A copy: sentences are added while these are read, and iterators into the map do not survive that.
    template<runir::kr::dl::CategoryTag Category>
    [[nodiscard]] urd::Store<urd::sequence<urd::index<runir::kr::dl::Constructor<Category>>>> get(NonTerminalView<Category> nonterminal,
                                                                                                  size_t complexity) const;
};

class Timeout
{
private:
    std::optional<ygg::CountdownWatch> m_stopwatch;

public:
    explicit Timeout(const GenerateOptions& options);

    [[nodiscard]] bool has_finished() const;
};

template<runir::kr::dl::CategoryTag Category>
struct SeenDenotationsFor
{
    using Constructor = runir::kr::dl::ConstructorView<Category>;
    using Denotation = runir::kr::dl::semantics::DenotationView<Category>;
    using NonTerminal = NonTerminalView<Category>;

    /// The denotation rows seen (one denotation per state), interned in a urd pool by the denotations' indices: they
    /// all live in the one canonical repository, so an index identifies a denotation. A handle's value identifies a row.
    urd::Store<urd::pool<urd::Store<urd::sequence<urd::index<runir::kr::dl::semantics::Denotation<Category>>>, urd::frozen>>> vectors;
    urd::Store<urd::sequence<urd::index<runir::kr::dl::semantics::Denotation<Category>>>> scratch_indices;
    urd::Store<urd::set<std::pair<NonTerminal, ygg::uint_t>>> signatures;
    urd::Store<urd::set<std::pair<NonTerminal, Constructor>>> constructors;
    urd::Store<urd::sequence<Denotation>> scratch;

    explicit SeenDenotationsFor(size_t num_states);
    ~SeenDenotationsFor();

    SeenDenotationsFor(const SeenDenotationsFor&) = delete;
    SeenDenotationsFor& operator=(const SeenDenotationsFor&) = delete;
    SeenDenotationsFor(SeenDenotationsFor&&) = delete;
    SeenDenotationsFor& operator=(SeenDenotationsFor&&) = delete;
};

struct SeenDenotations
{
    template<runir::kr::dl::CategoryTag Category>
    using Seen = SeenDenotationsFor<Category>;

    std::tuple<Seen<runir::kr::dl::ConceptTag>, Seen<runir::kr::dl::RoleTag>, Seen<runir::kr::dl::BooleanTag>, Seen<runir::kr::dl::NumericalTag>> values;

    explicit SeenDenotations(size_t num_states);
    ~SeenDenotations();

    SeenDenotations(const SeenDenotations&) = delete;
    SeenDenotations& operator=(const SeenDenotations&) = delete;
    SeenDenotations(SeenDenotations&&) = delete;
    SeenDenotations& operator=(SeenDenotations&&) = delete;

    template<runir::kr::dl::CategoryTag Category>
    SeenDenotationsFor<Category>& get() noexcept;
};

}

#endif
