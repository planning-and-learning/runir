#ifndef RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_DATA_HPP_
#define RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_DATA_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>
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

    Data() = default;
    Data(IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> concept_arguments_,
         IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> role_arguments_,
         IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> boolean_arguments_,
         IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> numerical_arguments_) :
        index(),
        concept_arguments(std::move(concept_arguments_)),
        role_arguments(std::move(role_arguments_)),
        boolean_arguments(std::move(boolean_arguments_)),
        numerical_arguments(std::move(numerical_arguments_))
    {
    }
    template<typename C>
    Data(const std::vector<::ygg::View<Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>, C>>& concept_arguments_,
         const std::vector<::ygg::View<Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>, C>>& role_arguments_,
         const std::vector<::ygg::View<Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>, C>>& boolean_arguments_,
         const std::vector<::ygg::View<Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>, C>>& numerical_arguments_) :
        index(),
        concept_arguments(),
        role_arguments(),
        boolean_arguments(),
        numerical_arguments()
    {
        set(concept_arguments_, concept_arguments);
        set(role_arguments_, role_arguments);
        set(boolean_arguments_, boolean_arguments);
        set(numerical_arguments_, numerical_arguments);
    }

    auto cista_members() noexcept { return std::tie(index, concept_arguments, role_arguments, boolean_arguments, numerical_arguments); }
    auto cista_members() const noexcept { return std::tie(index, concept_arguments, role_arguments, boolean_arguments, numerical_arguments); }
    auto identifying_members() const noexcept { return std::tie(concept_arguments, role_arguments, boolean_arguments, numerical_arguments); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
