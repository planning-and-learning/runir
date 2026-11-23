#ifndef RUNIR_KR_DL_GRAMMAR_GRAMMAR_BUILDER_HPP_
#define RUNIR_KR_DL_GRAMMAR_GRAMMAR_BUILDER_HPP_

#include "runir/kr/dl/grammar/grammar_data.hpp"
#include "runir/kr/dl/grammar/grammar_view.hpp"

#include <optional>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/planning/domain_builder.hpp>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of GrammarTag: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::grammar::GrammarTag, C>
{
    urd::key<runir::kr::dl::grammar::GrammarTag> index;
    urd::built_t<urd::flat_optional<urd::index<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::ConceptTag>>>, C> concept_start;
    urd::built_t<urd::flat_optional<urd::index<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::RoleTag>>>, C> role_start;
    urd::built_t<urd::flat_optional<urd::index<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::BooleanTag>>>, C> boolean_start;
    urd::built_t<urd::flat_optional<urd::index<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::NumericalTag>>>, C> numerical_start;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::grammar::DerivationRule<runir::kr::dl::ConceptTag>>>, C> concept_derivation_rules;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::grammar::DerivationRule<runir::kr::dl::RoleTag>>>, C> role_derivation_rules;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::grammar::DerivationRule<runir::kr::dl::BooleanTag>>>, C> boolean_derivation_rules;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::grammar::DerivationRule<runir::kr::dl::NumericalTag>>>, C> numerical_derivation_rules;
    urd::built_t<urd::index<::tyr::formalism::Domain>, ::tyr::formalism::Repository> domain;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"concept_start", &builder::concept_start>,
                           urd::field<"role_start", &builder::role_start>,
                           urd::field<"boolean_start", &builder::boolean_start>,
                           urd::field<"numerical_start", &builder::numerical_start>,
                           urd::field<"concept_derivation_rules", &builder::concept_derivation_rules>,
                           urd::field<"role_derivation_rules", &builder::role_derivation_rules>,
                           urd::field<"boolean_derivation_rules", &builder::boolean_derivation_rules>,
                           urd::field<"numerical_derivation_rules", &builder::numerical_derivation_rules>,
                           urd::field<"domain", &builder::domain>> {};
    }

    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&builder::concept_start,
                            &builder::role_start,
                            &builder::boolean_start,
                            &builder::numerical_start,
                            &builder::concept_derivation_rules,
                            &builder::role_derivation_rules,
                            &builder::boolean_derivation_rules,
                            &builder::numerical_derivation_rules,
                            &builder::domain> {};
    }
};

}  // namespace urd

#endif
