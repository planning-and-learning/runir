#ifndef RUNIR_CNF_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_CNF_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/cnf_grammar/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/datas.hpp"
#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/cnf_grammar/views.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/detail/constructor_repository.hpp"

#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::dl::cnf_grammar
{

using ConstructorSymbolRepository = boost::mp11::mp_apply<ygg::formalism::SymbolRepository, ConstructorRepositoryTypes>;

using Builder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, ConstructorRepositoryTypes>;

using runir::kr::dl::detail::get_repository;
using ygg::formalism::checkout;

template<typename T>
    requires ygg::formalism::SupportsSymbol<ConstructorRepository, T>
void prepare_for_insert(ConstructorRepository&, urd::data<T>& data)
{ canonicalize(data); }

using ygg::formalism::insert;
}

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::dl::detail
{

extern template class BasicConstructorRepositoryFactory<runir::kr::dl::cnf_grammar::ConstructorRepositoryTypes>;

}  // namespace runir::kr::dl::detail
#endif

#endif
