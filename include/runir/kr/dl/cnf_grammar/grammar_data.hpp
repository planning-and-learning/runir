#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/indices.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <tyr/formalism/planning/domain_index.hpp>

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

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(concept_start);
        ygg::clear(role_start);
        ygg::clear(boolean_start);
        ygg::clear(numerical_start);
        ygg::clear(concept_derivation_rules);
        ygg::clear(role_derivation_rules);
        ygg::clear(boolean_derivation_rules);
        ygg::clear(numerical_derivation_rules);
        ygg::clear(concept_substitution_rules);
        ygg::clear(role_substitution_rules);
        ygg::clear(boolean_substitution_rules);
        ygg::clear(numerical_substitution_rules);
        ygg::clear(domain);
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
};

}  // namespace ygg

#endif
