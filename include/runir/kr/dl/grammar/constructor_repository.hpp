#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/argument_builder.hpp"
#include "runir/kr/dl/boolean_builder.hpp"
#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/concept_builder.hpp"
#include "runir/kr/dl/constructor_builder.hpp"
#include "runir/kr/dl/detail/constructor_repository.hpp"
#include "runir/kr/dl/grammar/canonicalization.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"
#include "runir/kr/dl/grammar/derivation_rule_builder.hpp"
#include "runir/kr/dl/grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/grammar/derivation_rule_view.hpp"
#include "runir/kr/dl/grammar/grammar_builder.hpp"
#include "runir/kr/dl/grammar/grammar_data.hpp"
#include "runir/kr/dl/grammar/grammar_view.hpp"
#include "runir/kr/dl/grammar/non_terminal_builder.hpp"
#include "runir/kr/dl/grammar/non_terminal_data.hpp"
#include "runir/kr/dl/grammar/non_terminal_view.hpp"
#include "runir/kr/dl/numerical_builder.hpp"
#include "runir/kr/dl/numerical_data.hpp"
#include "runir/kr/dl/query_builder.hpp"
#include "runir/kr/dl/record_view.hpp"
#include "runir/kr/dl/register_builder.hpp"
#include "runir/kr/dl/role_builder.hpp"

#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::kr::dl::grammar
{

using Builder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, boost::mp11::mp_push_front<ConstructorRepositoryTypes, ConstructorRepository>>;

using ygg::formalism::checkout;

}  // namespace runir::kr::dl::grammar

// Its symbols' reads, finds and inserts instantiated once, where the library instantiates the repository (brokkr's
// include rule I7): a file that reads records through it completes none of its stores.
YGG_FORMALISM_SYMBOL_CONTEXT(extern, runir::kr::dl::grammar::ConstructorRepository, 58)

#endif
