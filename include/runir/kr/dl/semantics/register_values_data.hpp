#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_DATA_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_DATA_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"

#include <cista/containers/optional.h>
#include <cista/containers/pair.h>
#include <cista/containers/vector.h>
#include <cstddef>
#include <optional>
#include <tuple>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::dl::semantics::RegisterValues>
{
    urd::index<runir::kr::dl::semantics::RegisterValues> index;
    ::cista::offset::vector<::cista::optional<urd::index<tyr::formalism::Object>>> concept_values;
    ::cista::offset::vector<::cista::optional<::cista::pair<urd::index<tyr::formalism::Object>, urd::index<tyr::formalism::Object>>>> role_values;

    data() = default;
    data(::cista::offset::vector<::cista::optional<urd::index<tyr::formalism::Object>>> concept_values_,
         ::cista::offset::vector<::cista::optional<::cista::pair<urd::index<tyr::formalism::Object>, urd::index<tyr::formalism::Object>>>> role_values_) :
        index(),
        concept_values(std::move(concept_values_)),
        role_values(std::move(role_values_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::concept_values, &data::role_values> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::concept_values, &data::role_values> {}; }
};

}  // namespace urd

namespace runir::kr::dl::semantics
{

template<typename C>
void assign_register(urd::data<RegisterValues>& data, RegisterIdentifier<ConceptTag> identifier, urd::index_view<tyr::formalism::Object, C> value)
{
    data.concept_values.at(static_cast<size_t>(static_cast<ygg::uint_t>(identifier.value))) = value.index();
    data.index = {};
}

template<typename C>
void assign_register(urd::data<RegisterValues>& data,
                     RegisterIdentifier<RoleTag> identifier,
                     const std::pair<urd::index_view<tyr::formalism::Object, C>, urd::index_view<tyr::formalism::Object, C>>& value)
{
    data.role_values.at(static_cast<size_t>(static_cast<ygg::uint_t>(identifier.value))) = ::cista::pair(value.first.index(), value.second.index());
    data.index = {};
}

/// Replace mutable values while retaining buffers and invalidating published identity.
template<RegisterValuesViewConcept R>
urd::data<RegisterValues>& assign(urd::data<RegisterValues>& data, R source)
{
    if (&data != &source.data())
    {
        data.concept_values = source.data().concept_values;
        data.role_values = source.data().role_values;
    }
    data.index = {};
    return data;
}

}  // namespace runir::kr::dl::semantics

#endif
