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

using FamilyConstructorSymbolRepository = ygg::ApplyTypeListT<ygg::formalism::SymbolRepository, FamilyConstructorRepositoryTypes>;

using Builder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, FamilyConstructorRepositoryTypes>;

using BaseBuilder = Builder;
using ExtBuilder = Builder;
using UnsBuilder = Builder;

using runir::kr::dl::detail::get_repository;
using ygg::formalism::checkout;

template<typename T>
    requires ygg::formalism::SupportsSymbol<BasicConstructorRepository, T>
void prepare_for_insert(BasicConstructorRepository& repository, ygg::Data<T>& data)
{
    canonicalize(data);
    detail::prepare(data, repository);
}

using ygg::formalism::insert;
}

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::dl::detail
{

extern template class ConstructorRepositoryFactory<runir::kr::dl::FamilyConstructorRepositoryTypes>;
extern template class ConstructorRepositoryFactory<runir::kr::dl::FamilyConstructorRepositoryTypes>;
extern template class ConstructorRepositoryFactory<runir::kr::dl::FamilyConstructorRepositoryTypes>;

}  // namespace runir::kr::dl::detail
#endif

#endif
