#include "module.hpp"

#include <runir/serialization/serialization.hpp>
#include <runir/serialization/types.hpp>
#include <tyr/serialization/types.hpp>
#include <yggdrasil/python/serialization.hpp>

namespace runir::serialization
{
namespace
{
using AllRegisteredTypes = ygg::ConcatTypeListsT<RegisteredTypes,
    ygg::ApplyTypeListT<ygg::ConcatTypeListsT, ygg::MapTypeListT<HashableTypeList, tyr::serialization::SerializedTypes>>>;
using AllSerializedTypes = ygg::ConcatTypeListsT<SerializedTypes, tyr::serialization::SerializedTypes>;
using ProjectionTypes = ygg::ConcatTypeListsT<SerializedTypes, tyr::serialization::ProjectionTypes, ygg::TypeList<kr::ps::ext::ExecutionPhase>>;
}  // namespace

void bind_module_definitions(nb::module_& m)
{
    ygg::python::bind_serialization(m, AllRegisteredTypes {}, AllSerializedTypes {}, ProjectionTypes {});
}

}  // namespace runir::serialization
