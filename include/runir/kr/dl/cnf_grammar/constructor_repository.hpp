#ifndef RUNIR_CNF_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_CNF_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/cnf_grammar/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/datas.hpp"
#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/cnf_grammar/views.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <tyr/common/type_list.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/formalism/symbol_repository.hpp>
#include <utility>
#include <vector>

namespace runir::kr::dl::cnf_grammar
{

using ConceptTypes = tyr::MapTypeListT<Concept, runir::kr::dl::ConceptConstructorTags>;
using RoleTypes = tyr::MapTypeListT<Role, runir::kr::dl::RoleConstructorTags>;
using BooleanTypes = tyr::MapTypeListT<Boolean, runir::kr::dl::BooleanConstructorTags>;
using NumericalTypes = tyr::MapTypeListT<Numerical, runir::kr::dl::NumericalConstructorTags>;
using ConstructorTypes = tyr::MapTypeListT<Constructor, runir::kr::dl::CategoryTags>;
using NonTerminalTypes = tyr::MapTypeListT<NonTerminal, runir::kr::dl::CategoryTags>;
using DerivationRuleTypes = tyr::MapTypeListT<DerivationRule, runir::kr::dl::CategoryTags>;
using SubstitutionRuleTypes = tyr::MapTypeListT<SubstitutionRule, runir::kr::dl::CategoryTags>;
using GrammarTypes = tyr::TypeList<GrammarTag>;

using ConstructorRepositoryTypes = tyr::ConcatTypeListsT<ConceptTypes,
                                                         RoleTypes,
                                                         BooleanTypes,
                                                         NumericalTypes,
                                                         ConstructorTypes,
                                                         NonTerminalTypes,
                                                         DerivationRuleTypes,
                                                         SubstitutionRuleTypes,
                                                         GrammarTypes>;

using ConstructorSymbolRepository = tyr::ApplyTypeListT<tyr::formalism::SymbolRepository, ConstructorRepositoryTypes>;

class ConstructorRepository
{
private:
    ConstructorSymbolRepository m_symbol_repository;
    std::shared_ptr<const tyr::formalism::planning::Repository> m_planning_repository;
    size_t m_index;

    template<typename T>
    std::optional<tyr::View<tyr::Index<T>, ConstructorRepository>> find_with_hash(const tyr::Data<T>& data, size_t hash) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<T>(data, hash))
            return tyr::View<tyr::Index<T>, ConstructorRepository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<tyr::View<tyr::Index<T>, ConstructorRepository>, bool> get_or_create_with_hash(tyr::Data<T>& data, size_t hash)
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<T>(data, hash))
            return { tyr::View<tyr::Index<T>, ConstructorRepository>(*index, *this), false };

        const auto [index, success] = m_symbol_repository.template get_or_create_local_with_hash<T>(data, hash);
        return { tyr::View<tyr::Index<T>, ConstructorRepository>(index, *this), success };
    }

public:
    ConstructorRepository(size_t index, std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository) :
        m_symbol_repository(nullptr),
        m_planning_repository(std::move(planning_repository)),
        m_index(index)
    {
        assert(m_planning_repository);
        clear();
    }

    ConstructorRepository(const ConstructorRepository&) = delete;
    ConstructorRepository& operator=(const ConstructorRepository&) = delete;
    ConstructorRepository(ConstructorRepository&&) = delete;
    ConstructorRepository& operator=(ConstructorRepository&&) = delete;

    const auto& get_index() const noexcept { return m_index; }
    const auto& get_planning_repository() const noexcept
    {
        assert(m_planning_repository);
        return *m_planning_repository;
    }
    const auto& get_planning_repository_ptr() const noexcept { return m_planning_repository; }

    void clear() noexcept { m_symbol_repository.clear(); }

    template<typename T>
    std::optional<tyr::View<tyr::Index<T>, ConstructorRepository>> find(const tyr::Data<T>& data) const noexcept
    {
        return find_with_hash<T>(data, ConstructorSymbolRepository::template hash<T>(data));
    }

    template<typename T>
    std::pair<tyr::View<tyr::Index<T>, ConstructorRepository>, bool> get_or_create(tyr::Data<T>& data)
    {
        return get_or_create_with_hash<T>(data, ConstructorSymbolRepository::template hash<T>(data));
    }

    template<typename T>
    const tyr::Data<T>& operator[](tyr::Index<T> index) const noexcept
    {
        assert(m_symbol_repository.template is_local<T>(index));
        return m_symbol_repository.template at_local<T>(index);
    }

    template<typename T>
    size_t size() const noexcept
    {
        return m_symbol_repository.template local_size<T>();
    }

    template<typename T>
    const ConstructorRepository& get_canonical_context(tyr::Index<T>) const noexcept
    {
        return *this;
    }
};

using ConstructorRepositoryPtr = std::shared_ptr<ConstructorRepository>;

template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptView = tyr::View<tyr::Index<Concept<Tag>>, ConstructorRepository>;
template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptListView = tyr::View<tyr::IndexList<Concept<Tag>>, ConstructorRepository>;
template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptViewList = std::vector<ConceptView<Tag>>;

template<runir::kr::dl::RoleConstructorTag Tag>
using RoleView = tyr::View<tyr::Index<Role<Tag>>, ConstructorRepository>;
template<runir::kr::dl::RoleConstructorTag Tag>
using RoleListView = tyr::View<tyr::IndexList<Role<Tag>>, ConstructorRepository>;
template<runir::kr::dl::RoleConstructorTag Tag>
using RoleViewList = std::vector<RoleView<Tag>>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanView = tyr::View<tyr::Index<Boolean<Tag>>, ConstructorRepository>;
template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanListView = tyr::View<tyr::IndexList<Boolean<Tag>>, ConstructorRepository>;
template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanViewList = std::vector<BooleanView<Tag>>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalView = tyr::View<tyr::Index<Numerical<Tag>>, ConstructorRepository>;
template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalListView = tyr::View<tyr::IndexList<Numerical<Tag>>, ConstructorRepository>;
template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalViewList = std::vector<NumericalView<Tag>>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorView = tyr::View<tyr::Index<Constructor<Category>>, ConstructorRepository>;
template<runir::kr::dl::CategoryTag Category>
using ConstructorListView = tyr::View<tyr::IndexList<Constructor<Category>>, ConstructorRepository>;
template<runir::kr::dl::CategoryTag Category>
using ConstructorViewList = std::vector<ConstructorView<Category>>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalView = tyr::View<tyr::Index<NonTerminal<Category>>, ConstructorRepository>;
template<runir::kr::dl::CategoryTag Category>
using NonTerminalListView = tyr::View<tyr::IndexList<NonTerminal<Category>>, ConstructorRepository>;
template<runir::kr::dl::CategoryTag Category>
using NonTerminalViewList = std::vector<NonTerminalView<Category>>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleView = tyr::View<tyr::Index<DerivationRule<Category>>, ConstructorRepository>;
template<runir::kr::dl::CategoryTag Category>
using DerivationRuleListView = tyr::View<tyr::IndexList<DerivationRule<Category>>, ConstructorRepository>;
template<runir::kr::dl::CategoryTag Category>
using DerivationRuleViewList = std::vector<DerivationRuleView<Category>>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleView = tyr::View<tyr::Index<SubstitutionRule<Category>>, ConstructorRepository>;
template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleListView = tyr::View<tyr::IndexList<SubstitutionRule<Category>>, ConstructorRepository>;
template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleViewList = std::vector<SubstitutionRuleView<Category>>;

using GrammarView = tyr::View<tyr::Index<GrammarTag>, ConstructorRepository>;

inline const ConstructorRepository& get_repository(const ConstructorRepository& repository) noexcept { return repository; }
inline ConstructorRepository& get_repository(ConstructorRepository& repository) noexcept { return repository; }

}

#endif
