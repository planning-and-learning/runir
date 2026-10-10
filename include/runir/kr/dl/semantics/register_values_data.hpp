#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_DATA_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_DATA_HPP_

#include "runir/kr/dl/semantics/register_values_index.hpp"

#include <cista/containers/optional.h>
#include <cista/containers/pair.h>
#include <cista/containers/vector.h>
#include <cstddef>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
#include <yggdrasil/core/types.hpp>
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

    Data() = default;
    // No view constructor: register slots are optional object (pairs), for which yggdrasil provides no ygg::set overload.
    Data(::cista::offset::vector<::cista::optional<Index<tyr::formalism::Object>>> concept_values_,
         ::cista::offset::vector<::cista::optional<::cista::pair<Index<tyr::formalism::Object>, Index<tyr::formalism::Object>>>> role_values_) :
        index(),
        concept_values(std::move(concept_values_)),
        role_values(std::move(role_values_))
    {
    }

    auto cista_members() noexcept { return std::tie(index, concept_values, role_values); }
    auto cista_members() const noexcept { return std::tie(index, concept_values, role_values); }
    auto identifying_members() const noexcept { return std::tie(concept_values, role_values); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

namespace runir::kr::dl::semantics
{

inline void assign_register(ygg::Data<RegisterValues>& data, RegisterIdentifier<ConceptTag> identifier, tyr::formalism::planning::ObjectView value)
{
    data.concept_values.at(static_cast<size_t>(ygg::uint_t(identifier))) = value.get_index();
    ygg::clear(data.index);
}

inline void assign_register(ygg::Data<RegisterValues>& data,
                            RegisterIdentifier<RoleTag> identifier,
                            const std::pair<tyr::formalism::planning::ObjectView, tyr::formalism::planning::ObjectView>& value)
{
    data.role_values.at(static_cast<size_t>(ygg::uint_t(identifier))) = ::cista::pair(value.first.get_index(), value.second.get_index());
    ygg::clear(data.index);
}

}  // namespace runir::kr::dl::semantics

#endif
