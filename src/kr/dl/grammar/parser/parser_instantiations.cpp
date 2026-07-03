#include "parser_def.hpp"

namespace runir::kr::dl::grammar::parser
{

#define RUNIR_INSTANTIATE_ALIAS(name) BOOST_SPIRIT_INSTANTIATE(name, iterator_type, context_type)
#define RUNIR_DEFINE_INSTANTIATION_TYPES(Prefix, Family)                                                                                           \
    using Prefix##_concept_root_type = constructor_root_type<Family, runir::kr::dl::ConceptTag>;                                                    \
    using Prefix##_role_root_type = constructor_root_type<Family, runir::kr::dl::RoleTag>;                                                          \
    using Prefix##_boolean_root_type = constructor_root_type<Family, runir::kr::dl::BooleanTag>;                                                    \
    using Prefix##_numerical_root_type = constructor_root_type<Family, runir::kr::dl::NumericalTag>;                                                \
    using Prefix##_concept_type = constructor_type<Family, runir::kr::dl::ConceptTag>;                                                              \
    using Prefix##_role_type = constructor_type<Family, runir::kr::dl::RoleTag>;                                                                    \
    using Prefix##_boolean_type = constructor_type<Family, runir::kr::dl::BooleanTag>;                                                              \
    using Prefix##_numerical_type = constructor_type<Family, runir::kr::dl::NumericalTag>;                                                          \
    using Prefix##_concept_choice_type = choice_type<Family, runir::kr::dl::ConceptTag>;                                                            \
    using Prefix##_role_choice_type = choice_type<Family, runir::kr::dl::RoleTag>;                                                                  \
    using Prefix##_boolean_choice_type = choice_type<Family, runir::kr::dl::BooleanTag>;                                                            \
    using Prefix##_numerical_choice_type = choice_type<Family, runir::kr::dl::NumericalTag>;                                                        \
    using Prefix##_grammar_root_type = grammar_root_type<Family>;

RUNIR_DEFINE_INSTANTIATION_TYPES(base_family, runir::kr::BaseFamilyTag)
RUNIR_DEFINE_INSTANTIATION_TYPES(ext_family, runir::kr::ExtFamilyTag)
RUNIR_DEFINE_INSTANTIATION_TYPES(uns_family, runir::kr::UnsFamilyTag)

RUNIR_INSTANTIATE_ALIAS(base_family_concept_root_type)
RUNIR_INSTANTIATE_ALIAS(base_family_role_root_type)
RUNIR_INSTANTIATE_ALIAS(base_family_boolean_root_type)
RUNIR_INSTANTIATE_ALIAS(base_family_numerical_root_type)
RUNIR_INSTANTIATE_ALIAS(base_family_concept_type)
RUNIR_INSTANTIATE_ALIAS(base_family_role_type)
RUNIR_INSTANTIATE_ALIAS(base_family_boolean_type)
RUNIR_INSTANTIATE_ALIAS(base_family_numerical_type)
RUNIR_INSTANTIATE_ALIAS(base_family_concept_choice_type)
RUNIR_INSTANTIATE_ALIAS(base_family_role_choice_type)
RUNIR_INSTANTIATE_ALIAS(base_family_boolean_choice_type)
RUNIR_INSTANTIATE_ALIAS(base_family_numerical_choice_type)
RUNIR_INSTANTIATE_ALIAS(base_family_grammar_root_type)

RUNIR_INSTANTIATE_ALIAS(ext_family_concept_root_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_role_root_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_boolean_root_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_numerical_root_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_concept_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_role_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_boolean_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_numerical_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_concept_choice_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_role_choice_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_boolean_choice_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_numerical_choice_type)
RUNIR_INSTANTIATE_ALIAS(ext_family_grammar_root_type)

RUNIR_INSTANTIATE_ALIAS(uns_family_concept_root_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_role_root_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_boolean_root_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_numerical_root_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_concept_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_role_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_boolean_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_numerical_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_concept_choice_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_role_choice_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_boolean_choice_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_numerical_choice_type)
RUNIR_INSTANTIATE_ALIAS(uns_family_grammar_root_type)

#undef RUNIR_DEFINE_INSTANTIATION_TYPES
#undef RUNIR_INSTANTIATE_ALIAS

}  // namespace runir::kr::dl::grammar::parser
