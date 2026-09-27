#ifndef RUNIR_KR_PS_ICP_REPOSITORY_HPP_
#define RUNIR_KR_PS_ICP_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/icp/canonicalization.hpp"
#include "runir/kr/ps/icp/views.hpp"
#include "runir/kr/ps/repository.hpp"

namespace runir::kr::ps::icp
{

using Builder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, RepositoryTypes>;

template<typename T>
[[nodiscard]] auto checkout(Builder& builder)
{
    auto data = builder.template get_builder<T>();
    data->clear();
    return data;
}

template<typename T>
[[nodiscard]] auto get_or_create(Repository& repository, ygg::Data<T>& data)
{
    canonicalize(data);
    return repository.get_or_create(data);
}

}

#endif
