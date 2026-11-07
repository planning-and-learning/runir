#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include <cista/containers/optional.h>
#include <optional>
#include <tuple>
#include <tyr/formalism/planning/declarations.hpp>
#include <utility>
#include <vector>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::dl::cnf_grammar::Grammar>
{
    urd::index<runir::kr::dl::cnf_grammar::Grammar> index;
    urd::flat_optional<urd::index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>> concept_start;
    urd::flat_optional<urd::index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>> role_start;
    urd::flat_optional<urd::index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::BooleanTag>>> boolean_start;
    urd::flat_optional<urd::index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::NumericalTag>>> numerical_start;
    urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::dl::ConceptTag>>> concept_derivation_rules;
    urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::dl::RoleTag>>> role_derivation_rules;
    urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::dl::BooleanTag>>> boolean_derivation_rules;
    urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::dl::NumericalTag>>> numerical_derivation_rules;
    urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::dl::ConceptTag>>> concept_substitution_rules;
    urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::dl::RoleTag>>> role_substitution_rules;
    urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::dl::BooleanTag>>> boolean_substitution_rules;
    urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::dl::NumericalTag>>> numerical_substitution_rules;
    urd::index<::tyr::formalism::planning::Domain> domain;

    data() = default;
    data(urd::flat_optional<urd::index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>> concept_start_,
         urd::flat_optional<urd::index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>> role_start_,
         urd::flat_optional<urd::index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::BooleanTag>>> boolean_start_,
         urd::flat_optional<urd::index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::NumericalTag>>> numerical_start_,
         urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::dl::ConceptTag>>> concept_derivation_rules_,
         urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::dl::RoleTag>>> role_derivation_rules_,
         urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::dl::BooleanTag>>> boolean_derivation_rules_,
         urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::dl::NumericalTag>>> numerical_derivation_rules_,
         urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::dl::ConceptTag>>> concept_substitution_rules_,
         urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::dl::RoleTag>>> role_substitution_rules_,
         urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::dl::BooleanTag>>> boolean_substitution_rules_,
         urd::flat_vector<urd::index<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::dl::NumericalTag>>> numerical_substitution_rules_,
         urd::index<::tyr::formalism::planning::Domain> domain_) :
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
        domain(domain_)
    {
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
    [[nodiscard]] constexpr const auto& get_start() const noexcept
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
    [[nodiscard]] constexpr const auto& get_derivation_rules() const noexcept
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
    [[nodiscard]] constexpr const auto& get_substitution_rules() const noexcept
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

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&data::index,
                            &data::concept_start,
                            &data::role_start,
                            &data::boolean_start,
                            &data::numerical_start,
                            &data::concept_derivation_rules,
                            &data::role_derivation_rules,
                            &data::boolean_derivation_rules,
                            &data::numerical_derivation_rules,
                            &data::concept_substitution_rules,
                            &data::role_substitution_rules,
                            &data::boolean_substitution_rules,
                            &data::numerical_substitution_rules,
                            &data::domain> {};
    }
    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&data::concept_start,
                            &data::role_start,
                            &data::boolean_start,
                            &data::numerical_start,
                            &data::concept_derivation_rules,
                            &data::role_derivation_rules,
                            &data::boolean_derivation_rules,
                            &data::numerical_derivation_rules,
                            &data::concept_substitution_rules,
                            &data::role_substitution_rules,
                            &data::boolean_substitution_rules,
                            &data::numerical_substitution_rules,
                            &data::domain> {};
    }
};

}  // namespace urd

#endif
