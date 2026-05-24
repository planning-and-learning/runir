#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/grammar/canonicalization.hpp"
#include "runir/kr/dl/grammar/datas.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"
#include "runir/kr/dl/grammar/views.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <tyr/common/type_list.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/formalism/symbol_repository.hpp>
#include <utility>
#include <vector>

namespace runir::kr::dl::grammar
{

template<runir::kr::dl::FamilyTag Family>
using FamilyConceptTypes = tyr::MapTypeListSecondT<Concept, Family, runir::kr::dl::FamilyConceptConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyRoleTypes = tyr::MapTypeListSecondT<Role, Family, runir::kr::dl::FamilyRoleConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyBooleanTypes = tyr::MapTypeListSecondT<Boolean, Family, runir::kr::dl::FamilyBooleanConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyNumericalTypes = tyr::MapTypeListSecondT<Numerical, Family, runir::kr::dl::FamilyNumericalConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorTypes = tyr::MapTypeListSecondT<Constructor, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorOrNonTerminalTypes = tyr::MapTypeListSecondT<ConstructorOrNonTerminal, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyNonTerminalTypes = tyr::MapTypeListSecondT<NonTerminal, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyDerivationRuleTypes = tyr::MapTypeListSecondT<DerivationRule, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyGrammarTypes = tyr::TypeList<GrammarTag<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorRepositoryTypes = tyr::ConcatTypeListsT<FamilyConceptTypes<Family>,
                                                               FamilyRoleTypes<Family>,
                                                               FamilyBooleanTypes<Family>,
                                                               FamilyNumericalTypes<Family>,
                                                               FamilyConstructorTypes<Family>,
                                                               FamilyConstructorOrNonTerminalTypes<Family>,
                                                               FamilyNonTerminalTypes<Family>,
                                                               FamilyDerivationRuleTypes<Family>,
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

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryPtrFor = std::shared_ptr<ConstructorRepositoryFor<Family>>;

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

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
using FamilyConceptView = tyr::View<tyr::Index<Concept<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
using FamilyRoleView = tyr::View<tyr::Index<Role<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
using FamilyBooleanView = tyr::View<tyr::Index<Boolean<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
using FamilyNumericalView = tyr::View<tyr::Index<Numerical<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyConstructorView = tyr::View<tyr::Index<Constructor<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyConstructorOrNonTerminalView = tyr::View<tyr::Index<ConstructorOrNonTerminal<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyNonTerminalView = tyr::View<tyr::Index<NonTerminal<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyDerivationRuleView = tyr::View<tyr::Index<DerivationRule<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyGrammarView = tyr::View<tyr::Index<GrammarTag<Family>>, ConstructorRepositoryFor<Family>>;

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

}  // namespace runir::kr::dl::grammar

#endif
