#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_DATA_HPP_

#include "runir/cnf_grammar/indices.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <tyr/formalism/planning/domain_index.hpp>

namespace tyr
{

template<>
struct Data<runir::cnf_grammar::GrammarTag>
{
    using ConceptStart = ::cista::optional<Index<runir::cnf_grammar::NonTerminal<runir::ConceptTag>>>;
    using RoleStart = ::cista::optional<Index<runir::cnf_grammar::NonTerminal<runir::RoleTag>>>;
    using BooleanStart = ::cista::optional<Index<runir::cnf_grammar::NonTerminal<runir::BooleanTag>>>;
    using NumericalStart = ::cista::optional<Index<runir::cnf_grammar::NonTerminal<runir::NumericalTag>>>;

    Index<runir::cnf_grammar::GrammarTag> index;
    ConceptStart concept_start;
    RoleStart role_start;
    BooleanStart boolean_start;
    NumericalStart numerical_start;
    IndexList<runir::cnf_grammar::DerivationRule<runir::ConceptTag>> concept_derivation_rules;
    IndexList<runir::cnf_grammar::DerivationRule<runir::RoleTag>> role_derivation_rules;
    IndexList<runir::cnf_grammar::DerivationRule<runir::BooleanTag>> boolean_derivation_rules;
    IndexList<runir::cnf_grammar::DerivationRule<runir::NumericalTag>> numerical_derivation_rules;
    IndexList<runir::cnf_grammar::SubstitutionRule<runir::ConceptTag>> concept_substitution_rules;
    IndexList<runir::cnf_grammar::SubstitutionRule<runir::RoleTag>> role_substitution_rules;
    IndexList<runir::cnf_grammar::SubstitutionRule<runir::BooleanTag>> boolean_substitution_rules;
    IndexList<runir::cnf_grammar::SubstitutionRule<runir::NumericalTag>> numerical_substitution_rules;
    Index<formalism::planning::Domain> domain;

    Data() = default;

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(concept_start);
        tyr::clear(role_start);
        tyr::clear(boolean_start);
        tyr::clear(numerical_start);
        tyr::clear(concept_derivation_rules);
        tyr::clear(role_derivation_rules);
        tyr::clear(boolean_derivation_rules);
        tyr::clear(numerical_derivation_rules);
        tyr::clear(concept_substitution_rules);
        tyr::clear(role_substitution_rules);
        tyr::clear(boolean_substitution_rules);
        tyr::clear(numerical_substitution_rules);
        tyr::clear(domain);
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

}  // namespace tyr

#endif
