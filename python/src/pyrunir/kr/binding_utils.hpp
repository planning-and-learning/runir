#ifndef PYRUNIR_KR_BINDING_UTILS_HPP_
#define PYRUNIR_KR_BINDING_UTILS_HPP_

#include <yggdrasil/core/types.hpp>

namespace runir::kr::python
{

template<typename T, typename Repository>
auto get_or_create_data(Repository& repository, ygg::Data<T>& data)
{
    return repository.get_or_create(data).first;
}

}

#endif
