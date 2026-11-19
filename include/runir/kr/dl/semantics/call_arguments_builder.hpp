#ifndef RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_BUILDER_HPP_
#define RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_BUILDER_HPP_

#include "runir/kr/dl/semantics/call_arguments_data.hpp"
#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of CallArguments: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::semantics::CallArguments, C>
{
    urd::index<runir::kr::dl::semantics::CallArguments> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>>,
                 ::runir::kr::dl::semantics::DenotationRepository>
        concept_arguments;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>>, ::runir::kr::dl::semantics::DenotationRepository>
        role_arguments;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>>,
                 ::runir::kr::dl::semantics::DenotationRepository>
        boolean_arguments;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>>,
                 ::runir::kr::dl::semantics::DenotationRepository>
        numerical_arguments;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"concept_arguments", &builder::concept_arguments>,
                           urd::field<"role_arguments", &builder::role_arguments>,
                           urd::field<"boolean_arguments", &builder::boolean_arguments>,
                           urd::field<"numerical_arguments", &builder::numerical_arguments>> {};
    }

    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&builder::concept_arguments, &builder::role_arguments, &builder::boolean_arguments, &builder::numerical_arguments> {};
    }
};

}  // namespace urd

#endif
