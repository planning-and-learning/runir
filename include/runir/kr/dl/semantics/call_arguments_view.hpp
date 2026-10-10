#ifndef RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_VIEW_HPP_
#define RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_VIEW_HPP_

#include "runir/kr/dl/semantics/call_arguments_data.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/dependent_false.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::dl::semantics::CallArguments>, C> : public ygg::IndexViewBase<runir::kr::dl::semantics::CallArguments, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::semantics::CallArguments, C>::IndexViewBase;

    template<runir::kr::dl::CategoryTag Category>
    auto get() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(this->get_data().concept_arguments, this->get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return make_view(this->get_data().role_arguments, this->get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return make_view(this->get_data().boolean_arguments, this->get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return make_view(this->get_data().numerical_arguments, this->get_context());
        else
            static_assert(dependent_false<Category>::value, "unhandled argument category");
    }

    template<runir::kr::dl::CategoryTag Category>
    auto at(runir::kr::dl::ArgumentIdentifier<Category> arg) const
    {
        return get<Category>().at(static_cast<size_t>(ygg::uint_t(arg)));
    }

};

}  // namespace ygg

#endif
