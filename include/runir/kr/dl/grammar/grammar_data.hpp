#ifndef RUNIR_GRAMMAR_GRAMMAR_DATA_HPP_
#define RUNIR_GRAMMAR_GRAMMAR_DATA_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"
#include "runir/kr/dl/grammar/derivation_rule_data.hpp"

#include <cista/containers/optional.h>
#include <optional>
#include <tuple>
#include <tyr/formalism/planning/declarations.hpp>
#include <utility>
#include <vector>
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
    Data(::cista::optional<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>> concept_start_,
         ::cista::optional<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>> role_start_,
         ::cista::optional<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>>> boolean_start_,
         ::cista::optional<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>>> numerical_start_,
         IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::ConceptTag>> concept_derivation_rules_,
         IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::RoleTag>> role_derivation_rules_,
         IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::BooleanTag>> boolean_derivation_rules_,
         IndexList<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::NumericalTag>> numerical_derivation_rules_,
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
        domain(std::move(domain_))
    {
    }
    template<typename C, typename P>
    Data(const std::optional<::ygg::View<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>, C>>& concept_start_,
         const std::optional<::ygg::View<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>, C>>& role_start_,
         const std::optional<::ygg::View<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>>, C>>& boolean_start_,
         const std::optional<::ygg::View<Index<runir::kr::dl::grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>>, C>>& numerical_start_,
         const std::vector<::ygg::View<Index<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::ConceptTag>>, C>>& concept_derivation_rules_,
         const std::vector<::ygg::View<Index<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::RoleTag>>, C>>& role_derivation_rules_,
         const std::vector<::ygg::View<Index<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::BooleanTag>>, C>>& boolean_derivation_rules_,
         const std::vector<::ygg::View<Index<runir::kr::dl::grammar::DerivationRule<Family, runir::kr::dl::NumericalTag>>, C>>& numerical_derivation_rules_,
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
        set(domain_, domain);
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
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
