#ifndef RUNIR_KR_DL_SEMANTICS_INTERNING_HPP_
#define RUNIR_KR_DL_SEMANTICS_INTERNING_HPP_

#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <stdexcept>
#include <utility>

namespace runir::kr::dl::semantics
{

/// Extract mutable register values, retaining the destination's buffers and clearing its repository index.
template<RegisterValuesViewConcept R>
ygg::Data<RegisterValues>& make_data(R source, ygg::Data<RegisterValues>& data)
{
    data.concept_values = source.get_data().concept_values;
    data.role_values = source.get_data().role_values;
    ygg::clear(data.index);
    return data;
}

/// Intern a computed denotation using reusable scratch; record the resulting index on its builder.
template<CategoryTag Category>
[[nodiscard]] auto get_or_create(DenotationRepository& repository, ygg::Builder<Denotation<Category>>& source, Builder& builder)
{
    auto data = checkout<Denotation<Category>>(builder);
    if constexpr (ConceptOrRoleTag<Category>)
    {
        data->num_objects = source.num_objects;
        data->vec_index = repository.get_vector_repository().insert(source.blocks);
    }
    else
        data->value = source.value;
    auto interned = get_or_create(repository, *data);
    source.index = interned.first.get_index();
    return interned;
}

/// Borrowed registers contain object indices from the target's formalism repository.
[[nodiscard]] inline auto get_or_create(DenotationRepository& repository, BorrowedRegisterValuesView source, Builder& builder)
{
    if (&source.get_context() != &repository.get_formalism_repository())
        throw std::invalid_argument("Register interning requires the same formalism repository.");
    auto data = checkout<RegisterValues>(builder);
    make_data(source, *data);
    return get_or_create(repository, *data);
}

/// Reuse a view owned by the target; otherwise intern its values in the target repository.
[[nodiscard]] inline std::pair<RegisterValuesView, bool> get_or_create(DenotationRepository& repository, RegisterValuesView source, Builder& builder)
{
    if (&source.get_context() == &repository)
        return { source, false };
    return get_or_create(repository, BorrowedRegisterValuesView(source.get_data(), source.get_context().get_formalism_repository()), builder);
}

}  // namespace runir::kr::dl::semantics

#endif
