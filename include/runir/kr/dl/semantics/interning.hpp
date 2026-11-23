#ifndef RUNIR_KR_DL_SEMANTICS_INTERNING_HPP_
#define RUNIR_KR_DL_SEMANTICS_INTERNING_HPP_

#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <span>
#include <urd/elements/flat.hpp>

namespace runir::kr::dl::semantics
{

/// `destination` holding `source` (a denotation or a builder's view of one): a concept's or role's words copied into
/// its own (its buffer kept; `storage` holds no words of it).
template<CategoryTag Category, typename Source>
    requires DenotationViewConcept<Source, Category>
DenotationBuilder<Category>& assign(DenotationBuilder<Category>& destination, Source source, RecordStorage& /*storage*/)
{
    if constexpr (ConceptOrRoleTag<Category>)
    {
        const auto bits = source.get_values().bits();
        destination.num_objects = bits.num_objects();
        destination.values.assign(bits);
    }
    else
        destination.value = source.get_value();
    destination.index = {};
    return destination;
}

/// `source` under new columns (the same pool and rows) in the denotation repository's relation repository, from
/// `builder`.
[[nodiscard]] inline QueryDenotationView
rename(DenotationRepository& repository, QueryDenotationView source, std::span<const urd::data<ygg::ColumnLayout>> labels, Builder& builder)
{
    auto& relations = repository.get_relation_repository();
    auto& storage = builder.relations();
    auto relation = storage.template scratch<ygg::Relation<ygg::Schema>>();
    urd::store(relation->pool, source.get_pool());
    for (const auto row : source.get_rows())
        urd::append(relation->rows, row);
    auto columns = storage.template scratch<ygg::Columns>();
    columns->values.set(labels.begin(), labels.end());
    relation->columns = relations.insert(*columns).first.index();
    return relations.insert(*relation).first;
}

}  // namespace runir::kr::dl::semantics

#endif
