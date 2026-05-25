#ifndef RUNIR_KR_DL_EXT_REPOSITORY_HPP_
#define RUNIR_KR_DL_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/ext/datas.hpp"
#include "runir/kr/dl/ext/register_index.hpp"
#include "runir/kr/dl/repository.hpp"

namespace runir::kr::dl::ext
{

using ConstructorRepository = runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>;
using ConstructorRepositoryPtr = runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::ExtFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::ConstructorRepositoryFactoryFor<runir::kr::ExtFamilyTag>;

}  // namespace runir::kr::dl::ext

#endif
