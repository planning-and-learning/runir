#ifndef RUNIR_KR_PS_BASE_REPOSITORY_HPP_
#define RUNIR_KR_PS_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/base/canonicalization.hpp"
#include "runir/kr/ps/condition_data.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/dl/condition_data.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_data.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_data.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/effect_data.hpp"
#include "runir/kr/ps/base/rule_data.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_data.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/repository.hpp"

#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::ps::base
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
