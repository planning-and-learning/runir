#ifndef RUNIR_CNF_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_CNF_GRAMMAR_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/cnf_grammar/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/datas.hpp"
#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/cnf_grammar/views.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/detail/constructor_repository.hpp"

#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::dl::cnf_grammar
{

using FamilyConstructorSymbolRepository = ygg::ApplyTypeListT<ygg::formalism::SymbolRepository, FamilyConstructorRepositoryTypes>;

using Builder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, FamilyConstructorRepositoryTypes>;

using BaseBuilder = Builder;
using ExtBuilder = Builder;
using UnsBuilder = Builder;

using runir::kr::dl::detail::get_repository;
using ygg::formalism::checkout;

template<typename T>
    requires ygg::formalism::SupportsSymbol<BasicConstructorRepository, T>
void prepare_for_insert(BasicConstructorRepository&, ygg::Data<T>& data)
{
    canonicalize(data);
}

using ygg::formalism::insert;
}

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::dl::detail
{

extern template class ConstructorRepositoryFactory<runir::kr::dl::cnf_grammar::FamilyConstructorRepositoryTypes>;
extern template class ConstructorRepositoryFactory<runir::kr::dl::cnf_grammar::FamilyConstructorRepositoryTypes>;
extern template class ConstructorRepositoryFactory<runir::kr::dl::cnf_grammar::FamilyConstructorRepositoryTypes>;

}  // namespace runir::kr::dl::detail
#endif

#endif
