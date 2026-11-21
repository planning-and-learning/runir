#include "generation_state.hpp"

#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/cnf_grammar/generate.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <cstddef>
#include <optional>
#include <ranges>
#include <tuple>
#include <urd/store.hpp>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::dl::cnf_grammar::detail
{

GeneratedSentences::GeneratedSentences() = default;

GeneratedSentences::~GeneratedSentences() = default;

template<runir::kr::dl::CategoryTag Category>
void GeneratedSentences::add(NonTerminalView<Category> nonterminal, size_t complexity, urd::index<runir::kr::dl::Constructor<Category>> constructor)
{ std::get<Sentences<Category>>(m_sentences).insert({ { nonterminal.index(), complexity }, constructor }); }

template<runir::kr::dl::CategoryTag Category>
urd::Store<urd::sequence<urd::index<runir::kr::dl::Constructor<Category>>>> GeneratedSentences::get(NonTerminalView<Category> nonterminal,
                                                                                                    size_t complexity) const
{
    const auto [first, last] = std::get<Sentences<Category>>(m_sentences).equal_range({ nonterminal.index(), complexity });
    auto result = urd::Store<urd::sequence<urd::index<runir::kr::dl::Constructor<Category>>>>();
    for (const auto constructor : std::ranges::subrange(first, last) | std::views::values)
        result.push_back(constructor);
    return result;
}

Timeout::Timeout(const GenerateOptions& options) : m_stopwatch(options.max_time ? std::optional<ygg::CountdownWatch>(*options.max_time) : std::nullopt) {}

bool Timeout::has_finished() const { return m_stopwatch && m_stopwatch->has_finished(); }

template<runir::kr::dl::CategoryTag Category>
SeenDenotationsFor<Category>::SeenDenotationsFor(size_t num_states) : vectors(), scratch_indices(), signatures(), constructors(), scratch()
{
    scratch.reserve(num_states);
    scratch_indices.reserve(num_states);
}

template<runir::kr::dl::CategoryTag Category>
SeenDenotationsFor<Category>::~SeenDenotationsFor() = default;

SeenDenotations::SeenDenotations(size_t num_states) : values(num_states, num_states, num_states, num_states) {}

SeenDenotations::~SeenDenotations() = default;

template<runir::kr::dl::CategoryTag Category>
SeenDenotationsFor<Category>& SeenDenotations::get() noexcept
{ return std::get<Seen<Category>>(values); }

template struct SeenDenotationsFor<runir::kr::dl::ConceptTag>;
template struct SeenDenotationsFor<runir::kr::dl::RoleTag>;
template struct SeenDenotationsFor<runir::kr::dl::BooleanTag>;
template struct SeenDenotationsFor<runir::kr::dl::NumericalTag>;

template void GeneratedSentences::add<runir::kr::dl::ConceptTag>(NonTerminalView<runir::kr::dl::ConceptTag>,
                                                                 size_t,
                                                                 urd::index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>);
template void GeneratedSentences::add<runir::kr::dl::RoleTag>(NonTerminalView<runir::kr::dl::RoleTag>,
                                                              size_t,
                                                              urd::index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>);
template void GeneratedSentences::add<runir::kr::dl::BooleanTag>(NonTerminalView<runir::kr::dl::BooleanTag>,
                                                                 size_t,
                                                                 urd::index<runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>);
template void GeneratedSentences::add<runir::kr::dl::NumericalTag>(NonTerminalView<runir::kr::dl::NumericalTag>,
                                                                   size_t,
                                                                   urd::index<runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>>);

template urd::Store<urd::sequence<urd::index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>>>
GeneratedSentences::get<runir::kr::dl::ConceptTag>(NonTerminalView<runir::kr::dl::ConceptTag>, size_t) const;
template urd::Store<urd::sequence<urd::index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>>>
GeneratedSentences::get<runir::kr::dl::RoleTag>(NonTerminalView<runir::kr::dl::RoleTag>, size_t) const;
template urd::Store<urd::sequence<urd::index<runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>>>
GeneratedSentences::get<runir::kr::dl::BooleanTag>(NonTerminalView<runir::kr::dl::BooleanTag>, size_t) const;
template urd::Store<urd::sequence<urd::index<runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>>>>
GeneratedSentences::get<runir::kr::dl::NumericalTag>(NonTerminalView<runir::kr::dl::NumericalTag>, size_t) const;

template SeenDenotationsFor<runir::kr::dl::ConceptTag>& SeenDenotations::get<runir::kr::dl::ConceptTag>() noexcept;
template SeenDenotationsFor<runir::kr::dl::RoleTag>& SeenDenotations::get<runir::kr::dl::RoleTag>() noexcept;
template SeenDenotationsFor<runir::kr::dl::BooleanTag>& SeenDenotations::get<runir::kr::dl::BooleanTag>() noexcept;
template SeenDenotationsFor<runir::kr::dl::NumericalTag>& SeenDenotations::get<runir::kr::dl::NumericalTag>() noexcept;

}
