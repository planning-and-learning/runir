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

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorSymbolRepository = ygg::ApplyTypeListT<ygg::formalism::SymbolRepository, FamilyConstructorRepositoryTypes<Family>>;

template<runir::kr::dl::FamilyTag Family>
using Builder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, FamilyConstructorRepositoryTypes<Family>>;

using BaseBuilder = Builder<runir::kr::BaseFamilyTag>;
using ExtBuilder = Builder<runir::kr::ExtFamilyTag>;
using UnsBuilder = Builder<runir::kr::UnsFamilyTag>;

using runir::kr::dl::detail::get_repository;
using ygg::formalism::checkout;

template<runir::kr::dl::FamilyTag Family, typename T>
    requires ygg::formalism::SupportsSymbol<BasicConstructorRepository<Family>, T>
void prepare_for_insert(BasicConstructorRepository<Family>&, ygg::Data<T>& data)
{
    canonicalize(data);
}

using ygg::formalism::insert;
}

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::dl::detail
{

extern template class ConstructorRepositoryFactory<runir::kr::BaseFamilyTag,
                                                   runir::kr::dl::cnf_grammar::FamilyConstructorRepositoryTypes<runir::kr::BaseFamilyTag>>;
extern template class ConstructorRepositoryFactory<runir::kr::ExtFamilyTag,
                                                   runir::kr::dl::cnf_grammar::FamilyConstructorRepositoryTypes<runir::kr::ExtFamilyTag>>;
extern template class ConstructorRepositoryFactory<runir::kr::UnsFamilyTag,
                                                   runir::kr::dl::cnf_grammar::FamilyConstructorRepositoryTypes<runir::kr::UnsFamilyTag>>;

}  // namespace runir::kr::dl::detail
#endif

#endif
