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
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::dl::cnf_grammar
{

template<runir::kr::dl::FamilyTag Family>
struct RepositoryConstructorFamily
{
    template<typename Tag>
        requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
    using Concept = runir::kr::dl::cnf_grammar::Concept<Family, Tag>;

    template<typename Tag>
        requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
    using Role = runir::kr::dl::cnf_grammar::Role<Family, Tag>;

    template<typename Tag>
        requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
    using Boolean = runir::kr::dl::cnf_grammar::Boolean<Family, Tag>;

    template<typename Tag>
        requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
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
using FamilyConceptTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template Concept, runir::kr::dl::FamilyConceptConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyRoleTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template Role, runir::kr::dl::FamilyRoleConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyBooleanTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template Boolean, runir::kr::dl::FamilyBooleanConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyNumericalTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template Numerical, runir::kr::dl::FamilyNumericalConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template Constructor, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyNonTerminalTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template NonTerminal, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyDerivationRuleTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template DerivationRule, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilySubstitutionRuleTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template SubstitutionRule, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyGrammarTypes = ygg::TypeList<Grammar<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorRepositoryTypes = ygg::ConcatTypeListsT<FamilyConceptTypes<Family>,
                                                               FamilyRoleTypes<Family>,
                                                               FamilyBooleanTypes<Family>,
                                                               FamilyNumericalTypes<Family>,
                                                               FamilyConstructorTypes<Family>,
                                                               FamilyNonTerminalTypes<Family>,
                                                               FamilyDerivationRuleTypes<Family>,
                                                               FamilySubstitutionRuleTypes<Family>,
                                                               FamilyGrammarTypes<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorSymbolRepository = ygg::ApplyTypeListT<ygg::formalism::SymbolRepository, FamilyConstructorRepositoryTypes<Family>>;

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
    std::optional<ygg::View<ygg::Index<T>, BasicConstructorRepository>> find_with_hash(const ygg::Data<T>& data, size_t hash) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<T>(data, hash))
            return ygg::View<ygg::Index<T>, BasicConstructorRepository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<ygg::View<ygg::Index<T>, BasicConstructorRepository>, bool> get_or_create_with_hash(ygg::Data<T>& data, size_t hash)
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<T>(data, hash))
            return { ygg::View<ygg::Index<T>, BasicConstructorRepository>(*index, *this), false };

        const auto [index, success] = m_symbol_repository.template get_or_create_local_with_hash<T>(data, hash);
        return { ygg::View<ygg::Index<T>, BasicConstructorRepository>(index, *this), success };
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
    std::optional<ygg::View<ygg::Index<T>, BasicConstructorRepository>> find(const ygg::Data<T>& data) const noexcept
    {
        return find_with_hash<T>(data, FamilyConstructorSymbolRepository<Family>::template hash<T>(data));
    }

    template<typename T>
    std::pair<ygg::View<ygg::Index<T>, BasicConstructorRepository>, bool> get_or_create(ygg::Data<T>& data)
    {
        return get_or_create_with_hash<T>(data, FamilyConstructorSymbolRepository<Family>::template hash<T>(data));
    }

    template<typename T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const noexcept
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
    const BasicConstructorRepository& get_canonical_context(ygg::Index<T>) const noexcept
    {
        return *this;
    }
};

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryFor = BasicConstructorRepository<Family>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryPtrFor = std::shared_ptr<ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family>
class BasicConstructorRepositoryFactory
{
private:
    size_t m_next_index;

public:
    BasicConstructorRepositoryFactory() : m_next_index(0) {}

    ConstructorRepositoryPtrFor<Family> create(std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository)
    {
        return ConstructorRepositoryPtrFor<Family>(new ConstructorRepositoryFor<Family>(m_next_index++, std::move(planning_repository)));
    }
};

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryFactoryFor = BasicConstructorRepositoryFactory<Family>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
using FamilyConceptView = ygg::View<ygg::Index<Concept<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
using FamilyRoleView = ygg::View<ygg::Index<Role<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
using FamilyBooleanView = ygg::View<ygg::Index<Boolean<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
using FamilyNumericalView = ygg::View<ygg::Index<Numerical<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyConstructorView = ygg::View<ygg::Index<Constructor<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyNonTerminalView = ygg::View<ygg::Index<NonTerminal<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyDerivationRuleView = ygg::View<ygg::Index<DerivationRule<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilySubstitutionRuleView = ygg::View<ygg::Index<SubstitutionRule<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyGrammarView = ygg::View<ygg::Index<Grammar<Family>>, ConstructorRepositoryFor<Family>>;

using BaseConstructorRepository = ConstructorRepositoryFor<runir::kr::BaseFamilyTag>;
using BaseConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>;
using BaseGrammarView = FamilyGrammarView<runir::kr::BaseFamilyTag>;

using ExtConstructorRepository = ConstructorRepositoryFor<runir::kr::ExtFamilyTag>;
using ExtConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::ExtFamilyTag>;
using ExtGrammarView = FamilyGrammarView<runir::kr::ExtFamilyTag>;

using UnsConstructorRepository = ConstructorRepositoryFor<runir::kr::UnsFamilyTag>;
using UnsConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::UnsFamilyTag>;
using UnsGrammarView = FamilyGrammarView<runir::kr::UnsFamilyTag>;

template<typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseConceptView = FamilyConceptView<runir::kr::BaseFamilyTag, Tag>;

template<typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseRoleView = FamilyRoleView<runir::kr::BaseFamilyTag, Tag>;

template<typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseBooleanView = FamilyBooleanView<runir::kr::BaseFamilyTag, Tag>;

template<typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseNumericalView = FamilyNumericalView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::CategoryTag Category>
using BaseConstructorView = FamilyConstructorView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using BaseNonTerminalView = FamilyNonTerminalView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using BaseDerivationRuleView = FamilyDerivationRuleView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using BaseSubstitutionRuleView = FamilySubstitutionRuleView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::FamilyTag Family>
inline const ConstructorRepositoryFor<Family>& get_repository(const ConstructorRepositoryFor<Family>& repository) noexcept
{
    return repository;
}

template<runir::kr::dl::FamilyTag Family>
inline ConstructorRepositoryFor<Family>& get_repository(ConstructorRepositoryFor<Family>& repository) noexcept
{
    return repository;
}

}

#endif
