#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <cista/containers/optional.h>
#include <optional>
#include <tuple>
#include <tyr/formalism/declarations.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

namespace urd
{

template<>
struct data<runir::kr::dl::cnf_grammar::Grammar>
{
    urd::key<runir::kr::dl::cnf_grammar::Grammar> index;
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
    urd::index<::tyr::formalism::Domain> domain;

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
         urd::index<::tyr::formalism::Domain> domain_) :
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

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"concept_start", &data::concept_start>,
                           urd::field<"role_start", &data::role_start>,
                           urd::field<"boolean_start", &data::boolean_start>,
                           urd::field<"numerical_start", &data::numerical_start>,
                           urd::field<"concept_derivation_rules", &data::concept_derivation_rules>,
                           urd::field<"role_derivation_rules", &data::role_derivation_rules>,
                           urd::field<"boolean_derivation_rules", &data::boolean_derivation_rules>,
                           urd::field<"numerical_derivation_rules", &data::numerical_derivation_rules>,
                           urd::field<"concept_substitution_rules", &data::concept_substitution_rules>,
                           urd::field<"role_substitution_rules", &data::role_substitution_rules>,
                           urd::field<"boolean_substitution_rules", &data::boolean_substitution_rules>,
                           urd::field<"numerical_substitution_rules", &data::numerical_substitution_rules>,
                           urd::field<"domain", &data::domain>> {};
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

    [[nodiscard]] urd::key<runir::kr::dl::cnf_grammar::Grammar> key() const noexcept { return urd::key<runir::kr::dl::cnf_grammar::Grammar>(index); }
};

}  // namespace urd

namespace runir::kr::dl::cnf_grammar
{
/// The member of a grammar record (or of its builder, Data) holding the start non-terminal of a category.
template<CategoryTag Category, typename Data = urd::data<Grammar>>
consteval auto start_member() noexcept
{
    if constexpr (std::same_as<Category, ConceptTag>)
        return &Data::concept_start;
    else if constexpr (std::same_as<Category, RoleTag>)
        return &Data::role_start;
    else if constexpr (std::same_as<Category, BooleanTag>)
        return &Data::boolean_start;
    else
        return &Data::numerical_start;
}

/// The member of a grammar record (or of its builder, Data) holding the derivation rules of a category.
template<CategoryTag Category, typename Data = urd::data<Grammar>>
consteval auto derivation_rules_member() noexcept
{
    if constexpr (std::same_as<Category, ConceptTag>)
        return &Data::concept_derivation_rules;
    else if constexpr (std::same_as<Category, RoleTag>)
        return &Data::role_derivation_rules;
    else if constexpr (std::same_as<Category, BooleanTag>)
        return &Data::boolean_derivation_rules;
    else
        return &Data::numerical_derivation_rules;
}

/// The member of a grammar record (or of its builder, Data) holding the substitution rules of a category.
template<CategoryTag Category, typename Data = urd::data<Grammar>>
consteval auto substitution_rules_member() noexcept
{
    if constexpr (std::same_as<Category, ConceptTag>)
        return &Data::concept_substitution_rules;
    else if constexpr (std::same_as<Category, RoleTag>)
        return &Data::role_substitution_rules;
    else if constexpr (std::same_as<Category, BooleanTag>)
        return &Data::boolean_substitution_rules;
    else
        return &Data::numerical_substitution_rules;
}
}  // namespace runir::kr::dl::cnf_grammar

#endif
