#ifndef RUNIR_KR_DL_EXT_REPOSITORY_HPP_
#define RUNIR_KR_DL_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/ext/datas.hpp"
#include "runir/kr/dl/ext/register_index.hpp"
#include "runir/kr/dl/repository.hpp"

#include <memory>

namespace runir::kr::dl::ext
{

using ConstructorRepositoryTypes = runir::kr::dl::FamilyConstructorRepositoryTypes<runir::kr::dl::ExtFamilyTag>;

using ConstructorRepository = runir::kr::dl::ConstructorRepositoryFor<runir::kr::dl::ExtFamilyTag>;
using ConstructorRepositoryPtr = runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::dl::ExtFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::ConstructorRepositoryFactoryFor<runir::kr::dl::ExtFamilyTag>;
using ConstructorRepositoryFactoryPtr = std::shared_ptr<ConstructorRepositoryFactory>;

}  // namespace runir::kr::dl::ext

#endif
