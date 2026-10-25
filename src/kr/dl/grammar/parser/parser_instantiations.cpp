#include "runir/kr/dl/grammar/parser/parser_def.hpp"

namespace runir::kr::dl::grammar::parser
{

BOOST_SPIRIT_INSTANTIATE(dl_concept_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_role_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_boolean_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_numerical_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_concept_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_role_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_boolean_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_numerical_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_concept_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_role_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_boolean_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_numerical_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_query_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_grammar_root_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(dl_search_free_concept_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_role_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_boolean_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_numerical_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_concept_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_role_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_boolean_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_numerical_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_concept_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_role_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_boolean_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_numerical_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_query_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dl_search_free_grammar_root_type, iterator_type, context_type)

#define RUNIR_DEFINE_PARSER_ACCESSORS(Dialect, Prefix)                   \
    template<>                                                           \
    concept_type<Dialect> const& concept_parser<Dialect>()               \
    { return Prefix##_concept; }                                         \
    template<>                                                           \
    concept_root_type<Dialect> const& concept_root_parser<Dialect>()     \
    { return Prefix##_concept_root; }                                    \
    template<>                                                           \
    role_type<Dialect> const& role_parser<Dialect>()                     \
    { return Prefix##_role; }                                            \
    template<>                                                           \
    role_root_type<Dialect> const& role_root_parser<Dialect>()           \
    { return Prefix##_role_root; }                                       \
    template<>                                                           \
    boolean_type<Dialect> const& boolean_parser<Dialect>()               \
    { return Prefix##_boolean; }                                         \
    template<>                                                           \
    boolean_root_type<Dialect> const& boolean_root_parser<Dialect>()     \
    { return Prefix##_boolean_root; }                                    \
    template<>                                                           \
    numerical_type<Dialect> const& numerical_parser<Dialect>()           \
    { return Prefix##_numerical; }                                       \
    template<>                                                           \
    numerical_root_type<Dialect> const& numerical_root_parser<Dialect>() \
    { return Prefix##_numerical_root; }                                  \
    template<>                                                           \
    query_type<Dialect> const& query_parser<Dialect>()                   \
    { return Prefix##_query; }                                           \
    template<>                                                           \
    grammar_root_type<Dialect> const& grammar_root_parser<Dialect>()     \
    { return Prefix##_grammar_root; }

RUNIR_DEFINE_PARSER_ACCESSORS(FullDialectTag, dl)
RUNIR_DEFINE_PARSER_ACCESSORS(SearchFreeDialectTag, dl_search_free)

#undef RUNIR_DEFINE_PARSER_ACCESSORS

}  // namespace runir::kr::dl::grammar::parser
