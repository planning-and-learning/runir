#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_DATA_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_DATA_HPP_

#include "runir/kr/dl/semantics/register_values_index.hpp"

#include <cista/containers/optional.h>
#include <cista/containers/pair.h>
#include <cista/containers/vector.h>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/serialization/cista_equal_to.hpp>
#include <yggdrasil/serialization/cista_hash.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::dl::semantics::RegisterValues>
{
    Index<runir::kr::dl::semantics::RegisterValues> index;
    ::cista::offset::vector<::cista::optional<Index<tyr::formalism::Object>>> concept_values;
    ::cista::offset::vector<::cista::optional<::cista::pair<Index<tyr::formalism::Object>, Index<tyr::formalism::Object>>>> role_values;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(concept_values);
        ygg::clear(role_values);
    }

    auto cista_members() const noexcept { return std::tie(index, concept_values, role_values); }
    auto identifying_members() const noexcept { return std::tie(concept_values, role_values); }
};

}  // namespace ygg

#endif
