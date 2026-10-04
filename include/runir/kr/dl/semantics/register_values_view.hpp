#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_data.hpp"

#include <concepts>
#include <cstddef>
#include <tuple>
#include <tyr/formalism/object_view.hpp>
#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/pair.hpp>
#include <yggdrasil/containers/vector.hpp>

namespace ygg
{

/// Borrows the register data and formalism repository; both must outlive the view and its ranges.
template<typename C>
class View<Data<runir::kr::dl::semantics::RegisterValues>, C>
{
    const Data<runir::kr::dl::semantics::RegisterValues>* m_handle;
    const C* m_context;

public:
    View(const Data<runir::kr::dl::semantics::RegisterValues>& handle, const C& context) noexcept : m_handle(&handle), m_context(&context) {}

    const auto& get_data() const noexcept { return *m_handle; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return *m_handle; }
    const auto& get_formalism_repository() const noexcept { return *m_context; }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto get() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(get_data().concept_values, get_formalism_repository());
        else
            return make_view(get_data().role_values, get_formalism_repository());
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto at(runir::kr::dl::RegisterIdentifier<Category> identifier) const
    {
        return get<Category>().at(static_cast<size_t>(ygg::uint_t(identifier)));
    }
};

template<typename C>
class View<Index<runir::kr::dl::semantics::RegisterValues>, C>
{
private:
    Index<runir::kr::dl::semantics::RegisterValues> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::dl::semantics::RegisterValues> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const noexcept { return get_denotation_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }
    const auto& get_formalism_repository() const noexcept { return get_denotation_repository(*m_context).get_formalism_repository(); }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto get() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(get_data().concept_values, get_formalism_repository());
        else
            return make_view(get_data().role_values, get_formalism_repository());
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto at(runir::kr::dl::RegisterIdentifier<Category> reg) const
    {
        return get<Category>().at(static_cast<size_t>(ygg::uint_t(reg)));
    }

    auto identifying_members() const noexcept { return std::make_tuple(get_handle(), get_denotation_repository(*m_context).get_index()); }
};

}  // namespace ygg

namespace runir::kr::dl::semantics
{

template<typename V>
concept RegisterValuesViewConcept = requires(const V& values, RegisterIdentifier<ConceptTag> concept_id, RegisterIdentifier<RoleTag> role_id) {
    { values.get_data() } -> std::same_as<const ygg::Data<RegisterValues>&>;
    { values.get_formalism_repository() } -> std::same_as<const tyr::formalism::planning::Repository&>;
    values.template get<ConceptTag>();
    values.template get<RoleTag>();
    { values.at(concept_id).has_value() } -> std::same_as<bool>;
    { values.at(concept_id).value() } -> std::same_as<tyr::formalism::planning::ObjectView>;
    { values.at(role_id).has_value() } -> std::same_as<bool>;
    { values.at(role_id).value().get_first() } -> std::same_as<tyr::formalism::planning::ObjectView>;
    { values.at(role_id).value().get_second() } -> std::same_as<tyr::formalism::planning::ObjectView>;
};

}  // namespace runir::kr::dl::semantics

#endif
