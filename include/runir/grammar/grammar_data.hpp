#ifndef RUNIR_GRAMMAR_GRAMMAR_DATA_HPP_
#define RUNIR_GRAMMAR_GRAMMAR_DATA_HPP_

#include "runir/grammar/indices.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <tyr/formalism/planning/domain_index.hpp>

namespace tyr
{

template<>
struct Data<runir::grammar::GrammarTag>
{
    using ConceptStart = ::cista::optional<Index<runir::grammar::NonTerminal<runir::ConceptTag>>>;
    using RoleStart = ::cista::optional<Index<runir::grammar::NonTerminal<runir::RoleTag>>>;
    using BooleanStart = ::cista::optional<Index<runir::grammar::NonTerminal<runir::BooleanTag>>>;
    using NumericalStart = ::cista::optional<Index<runir::grammar::NonTerminal<runir::NumericalTag>>>;

    Index<runir::grammar::GrammarTag> index;
    ConceptStart concept_start;
    RoleStart role_start;
    BooleanStart boolean_start;
    NumericalStart numerical_start;
    IndexList<runir::grammar::DerivationRule<runir::ConceptTag>> concept_derivation_rules;
    IndexList<runir::grammar::DerivationRule<runir::RoleTag>> role_derivation_rules;
    IndexList<runir::grammar::DerivationRule<runir::BooleanTag>> boolean_derivation_rules;
    IndexList<runir::grammar::DerivationRule<runir::NumericalTag>> numerical_derivation_rules;
    Index<formalism::planning::Domain> domain;

    Data() = default;
    Data(ConceptStart concept_start_,
         RoleStart role_start_,
         BooleanStart boolean_start_,
         NumericalStart numerical_start_,
         IndexList<runir::grammar::DerivationRule<runir::ConceptTag>> concept_derivation_rules_,
         IndexList<runir::grammar::DerivationRule<runir::RoleTag>> role_derivation_rules_,
         IndexList<runir::grammar::DerivationRule<runir::BooleanTag>> boolean_derivation_rules_,
         IndexList<runir::grammar::DerivationRule<runir::NumericalTag>> numerical_derivation_rules_,
         Index<formalism::planning::Domain> domain_) :
        index(),
        concept_start(concept_start_),
        role_start(role_start_),
        boolean_start(boolean_start_),
        numerical_start(numerical_start_),
        concept_derivation_rules(std::move(concept_derivation_rules_)),
        role_derivation_rules(std::move(role_derivation_rules_)),
        boolean_derivation_rules(std::move(boolean_derivation_rules_)),
        numerical_derivation_rules(std::move(numerical_derivation_rules_)),
        domain(domain_)
    {
    }

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
                        domain);
    }
};

}  // namespace tyr

#endif
