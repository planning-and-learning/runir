#include "runir/kr/ps/base/repository.hpp"

#include "runir/kr/ps/detail/repository_impl.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/icp/execution_data.hpp"
#include "runir/kr/ps/icp/repository.hpp"
#include "runir/kr/uns/repository.hpp"

template class runir::kr::ps::BasicRepositoryFactory<runir::kr::BaseFamilyTag, runir::kr::ps::base::RepositoryTypes>;
template class runir::kr::ps::BasicRepositoryFactory<runir::kr::ExtFamilyTag, runir::kr::ps::ext::RepositoryTypes>;
template class runir::kr::ps::BasicRepositoryFactory<runir::kr::IcpFamilyTag, runir::kr::ps::icp::RepositoryTypes>;
template class runir::kr::ps::BasicRepositoryFactory<runir::kr::UnsFamilyTag, runir::kr::uns::RepositoryTypes>;

RUNIR_KR_PS_INTERNS()
