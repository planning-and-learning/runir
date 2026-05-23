#ifndef RUNIR_CNF_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_CNF_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/cnf_grammar/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/datas.hpp"
#include "runir/kr/dl/cnf_grammar/ext/datas.hpp"
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

template<runir::kr::dl::FamilyTag Family>
struct RepositoryConstructorFamily
{
    template<typename Tag>
        requires runir::kr::dl::ConceptConstructorTag<Tag>
    using Concept = runir::kr::dl::cnf_grammar::Concept<Family, Tag>;

    template<runir::kr::dl::RoleConstructorTag Tag>
    using Role = runir::kr::dl::cnf_grammar::Role<Family, Tag>;

    template<runir::kr::dl::BooleanConstructorTag Tag>
    using Boolean = runir::kr::dl::cnf_grammar::Boolean<Family, Tag>;

    template<runir::kr::dl::NumericalConstructorTag Tag>
    using Numerical = runir::kr::dl::cnf_grammar::Numerical<Family, Tag>;

    template<runir::kr::dl::CategoryTag Category>
    using Constructor = runir::kr::dl::cnf_grammar::Constructor<Family, Category>;

    template<runir::kr::dl::CategoryTag Category>
    using NonTerminal = runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>;

    template<runir::kr::dl::CategoryTag Category>
    using DerivationRule = runir::kr::dl::cnf_grammar::DerivationRule<Family, Category>;

    template<runir::kr::dl::CategoryTag Category>
    using SubstitutionRule = runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>;
};

template<runir::kr::dl::FamilyTag Family>
using FamilyConceptTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template Concept, runir::kr::dl::ConceptConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyRoleTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template Role, runir::kr::dl::RoleConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyBooleanTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template Boolean, runir::kr::dl::BooleanConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyNumericalTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template Numerical, runir::kr::dl::NumericalConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template Constructor, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyNonTerminalTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template NonTerminal, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyDerivationRuleTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template DerivationRule, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilySubstitutionRuleTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template SubstitutionRule, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyGrammarTypes = tyr::TypeList<Grammar<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorRepositoryTypes = tyr::ConcatTypeListsT<FamilyConceptTypes<Family>,
                                                              FamilyRoleTypes<Family>,
                                                              FamilyBooleanTypes<Family>,
                                                              FamilyNumericalTypes<Family>,
                                                              FamilyConstructorTypes<Family>,
                                                              FamilyNonTerminalTypes<Family>,
                                                              FamilyDerivationRuleTypes<Family>,
                                                              FamilySubstitutionRuleTypes<Family>,
                                                              FamilyGrammarTypes<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorSymbolRepository = tyr::ApplyTypeListT<tyr::formalism::SymbolRepository, FamilyConstructorRepositoryTypes<Family>>;

template<runir::kr::dl::FamilyTag Family>
class BasicConstructorRepository
{
    template<runir::kr::dl::FamilyTag>
    friend class BasicConstructorRepositoryFactory;

private:
    FamilyConstructorSymbolRepository<Family> m_symbol_repository;
    std::shared_ptr<const tyr::formalism::planning::Repository> m_planning_repository;
    size_t m_index;

    template<typename T>
    std::optional<tyr::View<tyr::Index<T>, BasicConstructorRepository>> find_with_hash(const tyr::Data<T>& data, size_t hash) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<T>(data, hash))
            return tyr::View<tyr::Index<T>, BasicConstructorRepository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<tyr::View<tyr::Index<T>, BasicConstructorRepository>, bool> get_or_create_with_hash(tyr::Data<T>& data, size_t hash)
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<T>(data, hash))
            return { tyr::View<tyr::Index<T>, BasicConstructorRepository>(*index, *this), false };

        const auto [index, success] = m_symbol_repository.template get_or_create_local_with_hash<T>(data, hash);
        return { tyr::View<tyr::Index<T>, BasicConstructorRepository>(index, *this), success };
    }

    BasicConstructorRepository(size_t index, std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository) :
        m_symbol_repository(nullptr),
        m_planning_repository(std::move(planning_repository)),
        m_index(index)
    {
        assert(m_planning_repository);
        clear();
    }

public:
    BasicConstructorRepository(const BasicConstructorRepository&) = delete;
    BasicConstructorRepository& operator=(const BasicConstructorRepository&) = delete;
    BasicConstructorRepository(BasicConstructorRepository&&) = delete;
    BasicConstructorRepository& operator=(BasicConstructorRepository&&) = delete;

    const auto& get_index() const noexcept { return m_index; }
    const auto& get_planning_repository() const noexcept
    {
        assert(m_planning_repository);
        return *m_planning_repository;
    }
    const auto& get_planning_repository_ptr() const noexcept { return m_planning_repository; }

    void clear() noexcept { m_symbol_repository.clear(); }

    template<typename T>
    std::optional<tyr::View<tyr::Index<T>, BasicConstructorRepository>> find(const tyr::Data<T>& data) const noexcept
    {
        return find_with_hash<T>(data, FamilyConstructorSymbolRepository<Family>::template hash<T>(data));
    }

    template<typename T>
    std::pair<tyr::View<tyr::Index<T>, BasicConstructorRepository>, bool> get_or_create(tyr::Data<T>& data)
    {
        return get_or_create_with_hash<T>(data, FamilyConstructorSymbolRepository<Family>::template hash<T>(data));
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
    const BasicConstructorRepository& get_canonical_context(tyr::Index<T>) const noexcept
    {
        return *this;
    }
};

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryFor = BasicConstructorRepository<Family>;

using ConstructorRepository = ConstructorRepositoryFor<runir::kr::BaseFamilyTag>;
using ExtConstructorRepository = ConstructorRepositoryFor<runir::kr::ExtFamilyTag>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryPtrFor = std::shared_ptr<ConstructorRepositoryFor<Family>>;

using ConstructorRepositoryPtr = ConstructorRepositoryPtrFor<runir::kr::BaseFamilyTag>;
using ExtConstructorRepositoryPtr = ConstructorRepositoryPtrFor<runir::kr::ExtFamilyTag>;

template<runir::kr::dl::FamilyTag Family>
class BasicConstructorRepositoryFactory
{
private:
    size_t m_next_index;

public:
    BasicConstructorRepositoryFactory() : m_next_index(0) {}

    ConstructorRepositoryFor<Family> create(std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository)
    {
        return ConstructorRepositoryFor<Family>(m_next_index++, std::move(planning_repository));
    }

    ConstructorRepositoryPtrFor<Family> create_shared(std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository)
    {
        return ConstructorRepositoryPtrFor<Family>(new ConstructorRepositoryFor<Family>(m_next_index++, std::move(planning_repository)));
    }
};

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryFactoryFor = BasicConstructorRepositoryFactory<Family>;

using ConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>;
using ExtConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::ExtFamilyTag>;

using ConstructorRepositoryFactoryPtr = std::shared_ptr<ConstructorRepositoryFactory>;
using ExtConstructorRepositoryFactoryPtr = std::shared_ptr<ExtConstructorRepositoryFactory>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
using FamilyConceptView = tyr::View<tyr::Index<Concept<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptView = FamilyConceptView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptListView = tyr::View<tyr::IndexList<Concept<runir::kr::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptViewList = std::vector<ConceptView<Tag>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag>
using FamilyRoleView = tyr::View<tyr::Index<Role<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::RoleConstructorTag Tag>
using RoleView = FamilyRoleView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::RoleConstructorTag Tag>
using RoleListView = tyr::View<tyr::IndexList<Role<runir::kr::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::RoleConstructorTag Tag>
using RoleViewList = std::vector<RoleView<Tag>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag>
using FamilyBooleanView = tyr::View<tyr::Index<Boolean<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanView = FamilyBooleanView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanListView = tyr::View<tyr::IndexList<Boolean<runir::kr::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanViewList = std::vector<BooleanView<Tag>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag>
using FamilyNumericalView = tyr::View<tyr::Index<Numerical<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalView = FamilyNumericalView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalListView = tyr::View<tyr::IndexList<Numerical<runir::kr::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalViewList = std::vector<NumericalView<Tag>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyConstructorView = tyr::View<tyr::Index<Constructor<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorView = FamilyConstructorView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorListView = tyr::View<tyr::IndexList<Constructor<runir::kr::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorViewList = std::vector<ConstructorView<Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyNonTerminalView = tyr::View<tyr::Index<NonTerminal<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalView = FamilyNonTerminalView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalListView = tyr::View<tyr::IndexList<NonTerminal<runir::kr::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalViewList = std::vector<NonTerminalView<Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyDerivationRuleView = tyr::View<tyr::Index<DerivationRule<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleView = FamilyDerivationRuleView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleListView = tyr::View<tyr::IndexList<DerivationRule<runir::kr::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleViewList = std::vector<DerivationRuleView<Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilySubstitutionRuleView = tyr::View<tyr::Index<SubstitutionRule<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleView = FamilySubstitutionRuleView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleListView = tyr::View<tyr::IndexList<SubstitutionRule<runir::kr::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleViewList = std::vector<SubstitutionRuleView<Category>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyGrammarView = tyr::View<tyr::Index<Grammar<Family>>, ConstructorRepositoryFor<Family>>;

using GrammarView = FamilyGrammarView<runir::kr::BaseFamilyTag>;

template<runir::kr::dl::FamilyTag Family>
inline const ConstructorRepositoryFor<Family>& get_repository(const ConstructorRepositoryFor<Family>& repository) noexcept { return repository; }

template<runir::kr::dl::FamilyTag Family>
inline ConstructorRepositoryFor<Family>& get_repository(ConstructorRepositoryFor<Family>& repository) noexcept { return repository; }

}

#endif
