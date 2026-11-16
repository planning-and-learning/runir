#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_DATA_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_DATA_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
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
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace urd
{

template<>
struct data<runir::kr::dl::semantics::RegisterValues>
{
    urd::index<runir::kr::dl::semantics::RegisterValues> index;
    urd::flat_vector<urd::flat_optional<urd::index<tyr::formalism::Object>>> concept_values;
    urd::flat_vector<urd::flat_optional<urd::flat_pair<urd::index<tyr::formalism::Object>, urd::index<tyr::formalism::Object>>>> role_values;

    data() = default;
    data(urd::flat_vector<urd::flat_optional<urd::index<tyr::formalism::Object>>> concept_values_,
         urd::flat_vector<urd::flat_optional<urd::flat_pair<urd::index<tyr::formalism::Object>, urd::index<tyr::formalism::Object>>>> role_values_) :
        index(),
        concept_values(std::move(concept_values_)),
        role_values(std::move(role_values_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"concept_values", &data::concept_values>, urd::field<"role_values", &data::role_values>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::concept_values, &data::role_values> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::concept_values, &data::role_values> {}; }
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

/// Replace mutable values while retaining buffers and invalidating published identity. A view of `data` itself (its
/// values read from data's own storage) leaves them as they are.
template<RegisterValuesViewConcept R>
urd::data<RegisterValues>& assign(urd::data<RegisterValues>& data, R source)
{
    const auto concept_values = source.template values<ConceptTag>();
    if (concept_values.base().data() != data.concept_values.data())
    {
        data.concept_values.clear();
        for (const auto& value : concept_values)
            urd::append(data.concept_values, value);
    }
    const auto role_values = source.template values<RoleTag>();
    if (role_values.base().data() != data.role_values.data())
    {
        data.role_values.clear();
        for (const auto& value : role_values)
            urd::append(data.role_values, value);
    }
    data.index = {};
    return data;
}

}  // namespace runir::kr::dl::semantics

#endif
