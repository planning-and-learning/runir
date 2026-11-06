#ifndef RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_DATA_HPP_
#define RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_DATA_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::dl::semantics::CallArguments>
{
    urd::index<runir::kr::dl::semantics::CallArguments> index;
    urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>> concept_arguments;
    urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>> role_arguments;
    urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>> boolean_arguments;
    urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>> numerical_arguments;

    data() = default;
    data(urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>> concept_arguments_,
         urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>> role_arguments_,
         urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>> boolean_arguments_,
         urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>> numerical_arguments_) :
        index(),
        concept_arguments(std::move(concept_arguments_)),
        role_arguments(std::move(role_arguments_)),
        boolean_arguments(std::move(boolean_arguments_)),
        numerical_arguments(std::move(numerical_arguments_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::concept_arguments, &data::role_arguments, &data::boolean_arguments, &data::numerical_arguments> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::concept_arguments, &data::role_arguments, &data::boolean_arguments, &data::numerical_arguments> {}; }
};

}  // namespace urd

#endif
