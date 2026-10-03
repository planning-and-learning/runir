#ifndef RUNIR_KR_DL_SEMANTICS_DENOTATION_STORAGE_HPP_
#define RUNIR_KR_DL_SEMANTICS_DENOTATION_STORAGE_HPP_

#include "runir/kr/dl/semantics/denotation_repository.hpp"

namespace runir::kr::dl::semantics
{

/// Intern a computed builder directly in its selected result repository.
template<CategoryTag Category>
auto intern_denotation(ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>& result, Builder& builder, DenotationRepository& repository)
{
    auto data = checkout<Denotation<Category>>(builder);
    make_data(*result, *data);
    if constexpr (ConceptOrRoleTag<Category>)
        data->vec_index = repository.get_vector_repository().insert(result->blocks);
    auto interned = get_or_create(repository, *data);
    result->index = interned.first.get_index();
    return interned;
}

}  // namespace runir::kr::dl::semantics

#endif
