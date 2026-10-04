#ifndef RUNIR_REPOSITORY_HPP_
#define RUNIR_REPOSITORY_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/construction_metadata.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/detail/constructor_repository.hpp"
#include "runir/kr/dl/query_construction.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"

#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::dl
{

template<FamilyTag Family>
using FamilyConstructorSymbolRepository = ygg::ApplyTypeListT<ygg::formalism::SymbolRepository, FamilyConstructorRepositoryTypes<Family>>;

template<FamilyTag Family>
using Builder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, FamilyConstructorRepositoryTypes<Family>>;

using BaseBuilder = Builder<runir::kr::BaseFamilyTag>;
using ExtBuilder = Builder<runir::kr::ExtFamilyTag>;
using UnsBuilder = Builder<runir::kr::UnsFamilyTag>;

using runir::kr::dl::detail::get_repository;
using ygg::formalism::checkout;

template<FamilyTag Family, typename T>
    requires ygg::formalism::SupportsSymbol<BasicConstructorRepository<Family>, T>
void prepare_for_interning(BasicConstructorRepository<Family>& repository, ygg::Data<T>& data)
{
    canonicalize(data);
    detail::prepare(data, repository);
}

using ygg::formalism::get_or_create;

}

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::dl::detail
{

extern template class ConstructorRepositoryFactory<runir::kr::BaseFamilyTag, runir::kr::dl::FamilyConstructorRepositoryTypes<runir::kr::BaseFamilyTag>>;
extern template class ConstructorRepositoryFactory<runir::kr::ExtFamilyTag, runir::kr::dl::FamilyConstructorRepositoryTypes<runir::kr::ExtFamilyTag>>;
extern template class ConstructorRepositoryFactory<runir::kr::UnsFamilyTag, runir::kr::dl::FamilyConstructorRepositoryTypes<runir::kr::UnsFamilyTag>>;

}  // namespace runir::kr::dl::detail
#endif

#endif
