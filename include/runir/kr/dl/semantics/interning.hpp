#ifndef RUNIR_KR_DL_SEMANTICS_INTERNING_HPP_
#define RUNIR_KR_DL_SEMANTICS_INTERNING_HPP_

#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <stdexcept>
#include <utility>

namespace runir::kr::dl::semantics
{

/// The target repository and scratch builder of a copy; brace-initialized at call sites as `{ repository, builder }`.
class CopyContext
{
    DenotationRepository* m_repository;
    Builder* m_builder;

public:
    CopyContext(DenotationRepository& repository, Builder& builder) noexcept : m_repository(&repository), m_builder(&builder) {}

    [[nodiscard]] DenotationRepository& repository() const noexcept { return *m_repository; }
    [[nodiscard]] Builder& builder() const noexcept { return *m_builder; }
};

template<CategoryTag Category>
urd::builder<Denotation<Category>>& assign(urd::builder<Denotation<Category>>& destination, DenotationView<Category> source)
{
    if constexpr (ConceptOrRoleTag<Category>)
    {
        const auto blocks = source.context().get_vector_repository().view(source.data().vec_index);
        destination.blocks.assign(blocks.begin(), blocks.end());
        destination.num_objects = source.data().num_objects;
    }
    else
        destination.value = source.value();
    destination.index = {};
    return destination;
}

template<CategoryTag Category>
urd::builder<Denotation<Category>>& assign(urd::builder<Denotation<Category>>& destination, BorrowedDenotationView<Category> source)
{
    if constexpr (ConceptOrRoleTag<Category>)
    {
        if (&destination != &source.data())
        {
            destination.blocks = source.data().blocks;
            destination.num_objects = source.get_num_objects();
        }
    }
    else
        destination.value = source.value();
    destination.index = {};
    return destination;
}

/// Intern a computed denotation using reusable scratch; record the resulting index on its builder.
template<CategoryTag Category>
[[nodiscard]] auto insert(DenotationRepository& repository, urd::builder<Denotation<Category>>& source, Builder& builder)
{
    auto data = checkout<Denotation<Category>>(builder);
    if constexpr (ConceptOrRoleTag<Category>)
    {
        data->num_objects = source.num_objects;
        data->vec_index = repository.get_vector_repository().insert(source.blocks).first;
    }
    else
        data->value = source.value;
    auto interned = insert(repository, *data);
    source.index = interned.first.index();
    return interned;
}

/// Publish typed query rows in the denotation repository's relation repository, which owns them.
[[nodiscard]] inline std::pair<QueryDenotationView, bool> insert(DenotationRepository& repository, urd::builder<ygg::database::Relation>& source)
{ return ygg::database::insert(repository.get_relation_repository(), source); }

/// Register object identities remain tied to the same planning formalism.
[[nodiscard]] inline auto copy(BorrowedRegisterValuesView source, const CopyContext& context)
{
    auto& repository = context.repository();
    auto& builder = context.builder();
    if (&source.get_formalism_repository() != &repository.get_formalism_repository())
        throw std::invalid_argument("Register copying requires the same formalism repository.");
    auto data = checkout<RegisterValues>(builder);
    assign(*data, source);
    return insert(repository, *data);
}

/// Reuse a view owned by the target; otherwise intern its values in the target repository.
[[nodiscard]] inline std::pair<RegisterValuesView, bool> copy(RegisterValuesView source, const CopyContext& context)
{
    if (&source.context() == &context.repository())
        return { source, false };
    return copy(BorrowedRegisterValuesView(&source.data(), &source.get_formalism_repository()), context);
}

template<CategoryTag Category>
[[nodiscard]] std::pair<DenotationView<Category>, bool> copy(DenotationView<Category> source, const CopyContext& context)
{
    auto& repository = context.repository();
    auto& builder = context.builder();
    if (&source.context() == &repository)
        return { source, false };
    auto data = checkout<Denotation<Category>>(builder);
    if constexpr (ConceptOrRoleTag<Category>)
    {
        if (source.context().get_formalism_repository_ptr() != repository.get_formalism_repository_ptr())
            throw std::invalid_argument("Denotation copying requires the same formalism repository.");
        data->num_objects = source.data().num_objects;
        const auto blocks = source.context().get_vector_repository().view(source.data().vec_index);
        data->vec_index = repository.get_vector_repository().insert(blocks).first;
    }
    else
        data->value = source.value();
    return insert(repository, *data);
}

}  // namespace runir::kr::dl::semantics

#endif
