#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/indices.hpp"

#include <cista/containers/optional.h>
#include <optional>
#include <tuple>
#include <tyr/formalism/planning/domain_index.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Grammar<Family>>
{
    using ConceptStart = ::cista::optional<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>>;
    using RoleStart = ::cista::optional<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>>;
    using BooleanStart = ::cista::optional<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>>>;
    using NumericalStart = ::cista::optional<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>>>;

    Index<runir::kr::dl::cnf_grammar::Grammar<Family>> index;
    ConceptStart concept_start;
    RoleStart role_start;
    BooleanStart boolean_start;
    NumericalStart numerical_start;
    IndexList<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::ConceptTag>> concept_derivation_rules;
    IndexList<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::RoleTag>> role_derivation_rules;
    IndexList<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::BooleanTag>> boolean_derivation_rules;
    IndexList<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::NumericalTag>> numerical_derivation_rules;
    IndexList<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::ConceptTag>> concept_substitution_rules;
    IndexList<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::RoleTag>> role_substitution_rules;
    IndexList<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::BooleanTag>> boolean_substitution_rules;
    IndexList<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::NumericalTag>> numerical_substitution_rules;
    Index<::tyr::formalism::planning::Domain> domain;

    Data() = default;
    Data(::cista::optional<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>> concept_start_,
         ::cista::optional<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>> role_start_,
         ::cista::optional<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>>> boolean_start_,
         ::cista::optional<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>>> numerical_start_,
         IndexList<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::ConceptTag>> concept_derivation_rules_,
         IndexList<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::RoleTag>> role_derivation_rules_,
         IndexList<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::BooleanTag>> boolean_derivation_rules_,
         IndexList<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::NumericalTag>> numerical_derivation_rules_,
         IndexList<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::ConceptTag>> concept_substitution_rules_,
         IndexList<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::RoleTag>> role_substitution_rules_,
         IndexList<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::BooleanTag>> boolean_substitution_rules_,
         IndexList<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::NumericalTag>> numerical_substitution_rules_,
         Index<::tyr::formalism::planning::Domain> domain_) :
        index(),
        concept_start(std::move(concept_start_)),
        role_start(std::move(role_start_)),
        boolean_start(std::move(boolean_start_)),
        numerical_start(std::move(numerical_start_)),
        concept_derivation_rules(std::move(concept_derivation_rules_)),
        role_derivation_rules(std::move(role_derivation_rules_)),
        boolean_derivation_rules(std::move(boolean_derivation_rules_)),
        numerical_derivation_rules(std::move(numerical_derivation_rules_)),
        concept_substitution_rules(std::move(concept_substitution_rules_)),
        role_substitution_rules(std::move(role_substitution_rules_)),
        boolean_substitution_rules(std::move(boolean_substitution_rules_)),
        numerical_substitution_rules(std::move(numerical_substitution_rules_)),
        domain(std::move(domain_))
    {
    }
    template<typename C, typename P>
    Data(const std::optional<::ygg::View<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>, C>>& concept_start_,
         const std::optional<::ygg::View<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>, C>>& role_start_,
         const std::optional<::ygg::View<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>>, C>>& boolean_start_,
         const std::optional<::ygg::View<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>>, C>>& numerical_start_,
         const std::vector<::ygg::View<Index<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::ConceptTag>>, C>>& concept_derivation_rules_,
         const std::vector<::ygg::View<Index<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::RoleTag>>, C>>& role_derivation_rules_,
         const std::vector<::ygg::View<Index<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::BooleanTag>>, C>>& boolean_derivation_rules_,
         const std::vector<::ygg::View<Index<runir::kr::dl::cnf_grammar::DerivationRule<Family, runir::kr::dl::NumericalTag>>, C>>& numerical_derivation_rules_,
         const std::vector<::ygg::View<Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::ConceptTag>>, C>>& concept_substitution_rules_,
         const std::vector<::ygg::View<Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::RoleTag>>, C>>& role_substitution_rules_,
         const std::vector<::ygg::View<Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::BooleanTag>>, C>>& boolean_substitution_rules_,
         const std::vector<::ygg::View<Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, runir::kr::dl::NumericalTag>>, C>>&
             numerical_substitution_rules_,
         ::ygg::View<Index<::tyr::formalism::planning::Domain>, P> domain_) :
        index(),
        concept_start(),
        role_start(),
        boolean_start(),
        numerical_start(),
        concept_derivation_rules(),
        role_derivation_rules(),
        boolean_derivation_rules(),
        numerical_derivation_rules(),
        concept_substitution_rules(),
        role_substitution_rules(),
        boolean_substitution_rules(),
        numerical_substitution_rules(),
        domain()
    {
        set(concept_start_, concept_start);
        set(role_start_, role_start);
        set(boolean_start_, boolean_start);
        set(numerical_start_, numerical_start);
        set(concept_derivation_rules_, concept_derivation_rules);
        set(role_derivation_rules_, role_derivation_rules);
        set(boolean_derivation_rules_, boolean_derivation_rules);
        set(numerical_derivation_rules_, numerical_derivation_rules);
        set(concept_substitution_rules_, concept_substitution_rules);
        set(role_substitution_rules_, role_substitution_rules);
        set(boolean_substitution_rules_, boolean_substitution_rules);
        set(numerical_substitution_rules_, numerical_substitution_rules);
        set(domain_, domain);
    }

    template<runir::kr::dl::CategoryTag Category>
    constexpr auto& get_start() noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_start;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return role_start;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return boolean_start;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return numerical_start;
    }

    template<runir::kr::dl::CategoryTag Category>
    constexpr const auto& get_start() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_start;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return role_start;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return boolean_start;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return numerical_start;
    }

    template<runir::kr::dl::CategoryTag Category>
    constexpr auto& get_derivation_rules() noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_derivation_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return role_derivation_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return boolean_derivation_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return numerical_derivation_rules;
    }

    template<runir::kr::dl::CategoryTag Category>
    constexpr const auto& get_derivation_rules() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_derivation_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return role_derivation_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return boolean_derivation_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return numerical_derivation_rules;
    }

    template<runir::kr::dl::CategoryTag Category>
    constexpr auto& get_substitution_rules() noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_substitution_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return role_substitution_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return boolean_substitution_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return numerical_substitution_rules;
    }

    template<runir::kr::dl::CategoryTag Category>
    constexpr const auto& get_substitution_rules() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_substitution_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return role_substitution_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return boolean_substitution_rules;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return numerical_substitution_rules;
    }

    auto cista_members() noexcept
    {
        return std::tie(index,
                        concept_start,
                        role_start,
                        boolean_start,
                        numerical_start,
                        concept_derivation_rules,
                        role_derivation_rules,
                        boolean_derivation_rules,
                        numerical_derivation_rules,
                        concept_substitution_rules,
                        role_substitution_rules,
                        boolean_substitution_rules,
                        numerical_substitution_rules,
                        domain);
    }
    auto cista_members() const noexcept
    {
        return std::tie(index,
                        concept_start,
                        role_start,
                        boolean_start,
                        numerical_start,
                        concept_derivation_rules,
                        role_derivation_rules,
                        boolean_derivation_rules,
                        numerical_derivation_rules,
                        concept_substitution_rules,
                        role_substitution_rules,
                        boolean_substitution_rules,
                        numerical_substitution_rules,
                        domain);
    }
    auto identifying_members() const noexcept
    {
        return std::tie(concept_start,
                        role_start,
                        boolean_start,
                        numerical_start,
                        concept_derivation_rules,
                        role_derivation_rules,
                        boolean_derivation_rules,
                        numerical_derivation_rules,
                        concept_substitution_rules,
                        role_substitution_rules,
                        boolean_substitution_rules,
                        numerical_substitution_rules,
                        domain);
    }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
