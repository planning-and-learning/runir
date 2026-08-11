#ifndef RUNIR_KR_UNS_REPOSITORY_HPP_
#define RUNIR_KR_UNS_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/repository.hpp"
#include "runir/kr/uns/canonicalization.hpp"
#include "runir/kr/uns/classifier_data.hpp"
#include "runir/kr/uns/classifier_view.hpp"
#include "runir/kr/uns/declarations.hpp"
#include "runir/kr/uns/dl/feature_data.hpp"
#include "runir/kr/uns/dl/feature_view.hpp"
#include "runir/kr/uns/feature_data.hpp"
#include "runir/kr/uns/feature_view.hpp"

#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::uns
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
