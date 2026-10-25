#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/object_view.hpp>
#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/pair.hpp>
#include <yggdrasil/containers/vector.hpp>

namespace ygg
{

/// Borrows the register data and formalism repository; both must outlive the view and its ranges.
template<typename C>
class View<Data<runir::kr::dl::semantics::RegisterValues>, C> : public ygg::DataViewBase<runir::kr::dl::semantics::RegisterValues, C>
{
public:
    using ygg::DataViewBase<runir::kr::dl::semantics::RegisterValues, C>::DataViewBase;

    const auto& get_formalism_repository() const noexcept { return this->get_context(); }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto get() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(this->get_data().concept_values, get_formalism_repository());
        else
            return make_view(this->get_data().role_values, get_formalism_repository());
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto at(runir::kr::dl::RegisterIdentifier<Category> identifier) const
    { return get<Category>().at(static_cast<size_t>(ygg::uint_t(identifier))); }
};

template<typename C>
class View<Index<runir::kr::dl::semantics::RegisterValues>, C> : public ygg::IndexViewBase<runir::kr::dl::semantics::RegisterValues, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::semantics::RegisterValues, C>::IndexViewBase;

    const auto& get_formalism_repository() const noexcept { return this->get_context().get_formalism_repository(); }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto get() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(this->get_data().concept_values, get_formalism_repository());
        else
            return make_view(this->get_data().role_values, get_formalism_repository());
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto at(runir::kr::dl::RegisterIdentifier<Category> reg) const
    { return get<Category>().at(static_cast<size_t>(ygg::uint_t(reg))); }
};

}  // namespace ygg

namespace runir::kr::dl::semantics
{

}  // namespace runir::kr::dl::semantics

#endif
