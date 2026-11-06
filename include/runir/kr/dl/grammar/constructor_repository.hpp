#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/detail/constructor_repository.hpp"
#include "runir/kr/dl/grammar/canonicalization.hpp"
#include "runir/kr/dl/grammar/datas.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"
#include "runir/kr/dl/grammar/views.hpp"

#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::dl::grammar
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

}  // namespace runir::kr::dl::grammar

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::dl::detail
{

extern template class BasicConstructorRepositoryFactory<runir::kr::dl::grammar::ConstructorRepositoryTypes>;

}  // namespace runir::kr::dl::detail
#endif

#endif
