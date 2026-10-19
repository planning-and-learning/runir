#include "runir/kr/dl/grammar/parser/parser_def.hpp"

namespace runir::kr::dl::grammar::parser
{

#define RUNIR_INSTANTIATE_ALIAS(name) BOOST_SPIRIT_INSTANTIATE(name, iterator_type, context_type)

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
RUNIR_INSTANTIATE_ALIAS(base_family_query_type)
RUNIR_INSTANTIATE_ALIAS(base_family_grammar_root_type)

RUNIR_INSTANTIATE_ALIAS(base_family_search_free_concept_root_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_role_root_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_boolean_root_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_numerical_root_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_concept_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_role_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_boolean_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_numerical_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_concept_choice_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_role_choice_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_boolean_choice_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_numerical_choice_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_query_type)
RUNIR_INSTANTIATE_ALIAS(base_family_search_free_grammar_root_type)

#undef RUNIR_INSTANTIATE_ALIAS

#define RUNIR_DEFINE_PARSER_ACCESSORS(Family, Dialect, Prefix)                           \
    template<>                                                                           \
    concept_type<Family, Dialect> const& concept_parser<Family, Dialect>()               \
    {                                                                                    \
        return Prefix##_concept;                                                         \
    }                                                                                    \
    template<>                                                                           \
    concept_root_type<Family, Dialect> const& concept_root_parser<Family, Dialect>()     \
    {                                                                                    \
        return Prefix##_concept_root;                                                    \
    }                                                                                    \
    template<>                                                                           \
    role_type<Family, Dialect> const& role_parser<Family, Dialect>()                     \
    {                                                                                    \
        return Prefix##_role;                                                            \
    }                                                                                    \
    template<>                                                                           \
    role_root_type<Family, Dialect> const& role_root_parser<Family, Dialect>()           \
    {                                                                                    \
        return Prefix##_role_root;                                                       \
    }                                                                                    \
    template<>                                                                           \
    boolean_type<Family, Dialect> const& boolean_parser<Family, Dialect>()               \
    {                                                                                    \
        return Prefix##_boolean;                                                         \
    }                                                                                    \
    template<>                                                                           \
    boolean_root_type<Family, Dialect> const& boolean_root_parser<Family, Dialect>()     \
    {                                                                                    \
        return Prefix##_boolean_root;                                                    \
    }                                                                                    \
    template<>                                                                           \
    numerical_type<Family, Dialect> const& numerical_parser<Family, Dialect>()           \
    {                                                                                    \
        return Prefix##_numerical;                                                       \
    }                                                                                    \
    template<>                                                                           \
    numerical_root_type<Family, Dialect> const& numerical_root_parser<Family, Dialect>() \
    {                                                                                    \
        return Prefix##_numerical_root;                                                  \
    }                                                                                    \
    template<>                                                                           \
    query_type<Family, Dialect> const& query_parser<Family, Dialect>()                   \
    {                                                                                    \
        return Prefix##_query;                                                           \
    }                                                                                    \
    template<>                                                                           \
    grammar_root_type<Family, Dialect> const& grammar_root_parser<Family, Dialect>()     \
    {                                                                                    \
        return Prefix##_grammar_root;                                                    \
    }

RUNIR_DEFINE_PARSER_ACCESSORS(runir::kr::BaseFamilyTag, FullDialectTag, base_family)
RUNIR_DEFINE_PARSER_ACCESSORS(runir::kr::BaseFamilyTag, SearchFreeDialectTag, base_family_search_free)

#undef RUNIR_DEFINE_PARSER_ACCESSORS

}  // namespace runir::kr::dl::grammar::parser
