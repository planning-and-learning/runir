#ifndef RUNIR_GRAMMAR_GRAMMAR_DATA_HPP_
#define RUNIR_GRAMMAR_GRAMMAR_DATA_HPP_

#include "runir/kr/dl/grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/grammar/grammar_index.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/formalism/planning/domain_index.hpp>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::GrammarTag<Family>>
{
    using ConceptStart = ::cista::optional<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>>;
    using RoleStart = ::cista::optional<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>>;
    using BooleanStart = ::cista::optional<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>>>;
    using NumericalStart = ::cista::optional<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>>>;

    Index<runir::kr::dl::grammar::GrammarTag<Family>> index;
    ConceptStart concept_start;
    RoleStart role_start;
    BooleanStart boolean_start;
    NumericalStart numerical_start;
    IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::ConceptTag>> concept_derivation_rules;
    IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::RoleTag>> role_derivation_rules;
    IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::BooleanTag>> boolean_derivation_rules;
    IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::NumericalTag>> numerical_derivation_rules;
    Index<::tyr::formalism::planning::Domain> domain;

    Data() = default;
    Data(ConceptStart concept_start_,
         RoleStart role_start_,
         BooleanStart boolean_start_,
         NumericalStart numerical_start_,
         IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::ConceptTag>> concept_derivation_rules_,
         IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::RoleTag>> role_derivation_rules_,
         IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::BooleanTag>> boolean_derivation_rules_,
         IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::NumericalTag>> numerical_derivation_rules_,
         Index<::tyr::formalism::planning::Domain> domain_) :
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
        ygg::clear(index);
        ygg::clear(concept_start);
        ygg::clear(role_start);
        ygg::clear(boolean_start);
        ygg::clear(numerical_start);
        ygg::clear(concept_derivation_rules);
        ygg::clear(role_derivation_rules);
        ygg::clear(boolean_derivation_rules);
        ygg::clear(numerical_derivation_rules);
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

}  // namespace ygg

#endif
