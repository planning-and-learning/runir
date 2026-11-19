#ifndef RUNIR_KR_DL_SEMANTICS_INTERNING_HPP_
#define RUNIR_KR_DL_SEMANTICS_INTERNING_HPP_

#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <cstddef>
#include <ranges>
#include <span>
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

/// `destination` holding `source` (a denotation or a builder's view of one): a concept's or role's words copied into
/// its own, drawn from `storage`, read through `repository`.
template<CategoryTag Category, typename Source>
    requires DenotationViewConcept<Source, Category>
DenotationBuilder<Category>& assign(DenotationBuilder<Category>& destination, Source source, RecordStorage& storage, const DenotationRepository& repository)
{
    if constexpr (ConceptOrRoleTag<Category>)
    {
        destination.num_objects = source.get_num_objects();
        detail::own_copied_words(destination.blocks, source.get_blocks(), storage, repository);
    }
    else
        destination.value = source.get_value();
    destination.index = {};
    return destination;
}

/// Intern a computed denotation; record the resulting index on its builder.
template<CategoryTag Category>
[[nodiscard]] auto insert(DenotationRepository& repository, DenotationBuilder<Category>& source, Builder& builder)
{
    auto interned = repository.insert(source, builder.records());
    source.index = interned.first.index();
    return interned;
}

/// These columns, interned into `relations` from a builder of `storage`.
[[nodiscard]] inline urd::index<ygg::Columns> interned_columns(ygg::FreeRelationRepository& relations, RelationStorage& storage, std::span<const ygg::ColumnLayout> layouts)
{
    auto columns = storage.template checkout<ygg::Columns>();
    columns->values.assign(layouts.begin(), layouts.end());
    return relations.insert(*columns, storage).first.index();
}

/// Publish a scratch relation's rows in the denotation repository's relation repository, which owns them: each row and
/// the columns from their builders, then the relation record (ygg interns no relation builder), from `builder`.
[[nodiscard]] inline std::pair<QueryDenotationView, bool> insert(DenotationRepository& repository, ygg::FreeRelationBuilderView source, Builder& builder)
{
    auto& relations = repository.get_relation_repository();
    auto& storage = builder.relations();
    const auto layouts = source.columns();
    const auto pool = relations.view(relations.schema(layouts));
    auto relation = storage.template scratch<ygg::Relation<ygg::Schema>>();
    relation->pool = pool.index();
    auto row = storage.template checkout<ygg::Row<ygg::Schema>>();
    row->relation = pool;
    for (std::size_t i = 0; i < source.size(); ++i)
    {
        ygg::with_values(source.row(i), [&row](const auto& values) { row->values.assign(std::ranges::begin(values), std::ranges::end(values)); });
        urd::append(relation->rows, relations.insert(*row, storage).first);
    }
    relation->columns = interned_columns(relations, storage, layouts);
    return relations.insert(*relation);
}

/// `source` under new columns (the same pool and rows) in the denotation repository's relation repository, from
/// `builder`.
[[nodiscard]] inline QueryDenotationView
rename(DenotationRepository& repository, QueryDenotationView source, std::span<const ygg::ColumnLayout> labels, Builder& builder)
{
    auto& relations = repository.get_relation_repository();
    auto& storage = builder.relations();
    auto relation = storage.template scratch<ygg::Relation<ygg::Schema>>();
    urd::store(relation->pool, source.get_pool());
    for (const auto row : source.get_rows())
        urd::append(relation->rows, row);
    relation->columns = interned_columns(relations, storage, labels);
    return relations.insert(*relation).first;
}

/// Register object identities remain tied to the same planning formalism. Registers read through any view but an
/// interned one's (a borrowed record's, a builder's) are interned from their values.
template<RegisterValuesViewConcept R>
    requires(!std::same_as<R, RegisterValuesView>)
[[nodiscard]] auto copy(R source, const CopyContext& context)
{
    auto& repository = context.repository();
    auto& builder = context.builder();
    if (&source.get_formalism_repository() != &repository.get_formalism_repository())
        throw std::invalid_argument("Register copying requires the same formalism repository.");
    auto data = builder.template scratch<RegisterValues>();
    assign(*data, source);
    return insert(repository, *data);
}

/// Reuse a view owned by the target; otherwise intern its values in the target repository.
[[nodiscard]] inline std::pair<RegisterValuesView, bool> copy(RegisterValuesView source, const CopyContext& context)
{
    if (&source.context() == &context.repository())
        return { source, false };
    return copy(RegisterValuesDataView(&source.context()[source.index()], &source.context()), context);
}

/// Reuse a view owned by the target; otherwise intern its values in the target repository.
template<CategoryTag Category>
[[nodiscard]] std::pair<DenotationView<Category>, bool> copy(DenotationView<Category> source, const CopyContext& context)
{
    auto& repository = context.repository();
    auto& builder = context.builder();
    if (&source.context() == &repository)
        return { source, false };
    if constexpr (ConceptOrRoleTag<Category>)
        if (source.context().get_formalism_repository_ptr() != repository.get_formalism_repository_ptr())
            throw std::invalid_argument("Denotation copying requires the same formalism repository.");
    auto copied = builder.template get_builder<Denotation<Category>>();
    assign(*copied, source, builder.records(), repository);
    return repository.insert(*copied, builder.records());
}

}  // namespace runir::kr::dl::semantics

#endif
