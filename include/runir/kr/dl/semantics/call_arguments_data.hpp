#ifndef RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_DATA_HPP_
#define RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_DATA_HPP_

#include "runir/kr/dl/semantics/call_arguments_index.hpp"
#include "runir/kr/dl/semantics/denotation_index.hpp"

#include <tuple>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/serialization/cista_equal_to.hpp>
#include <yggdrasil/serialization/cista_hash.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::dl::semantics::CallArguments>
{
    Index<runir::kr::dl::semantics::CallArguments> index;
    IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> concept_arguments;
    IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> role_arguments;
    IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> boolean_arguments;
    IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> numerical_arguments;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(concept_arguments);
        ygg::clear(role_arguments);
        ygg::clear(boolean_arguments);
        ygg::clear(numerical_arguments);
    }

    auto cista_members() const noexcept { return std::tie(index, concept_arguments, role_arguments, boolean_arguments, numerical_arguments); }
    auto identifying_members() const noexcept { return std::tie(concept_arguments, role_arguments, boolean_arguments, numerical_arguments); }
};

}  // namespace ygg

#endif
