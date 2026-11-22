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

namespace runir::kr::dl::grammar
{

using Builder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, boost::mp11::mp_push_front<ConstructorRepositoryTypes, ConstructorRepository>>;

using runir::kr::dl::detail::get_repository;
using ygg::formalism::checkout;


}  // namespace runir::kr::dl::grammar

#endif
