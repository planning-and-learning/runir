#ifndef RUNIR_KR_DL_SEMANTICS_BUILDER_HPP_
#define RUNIR_KR_DL_SEMANTICS_BUILDER_HPP_

#include "runir/kr/dl/semantics/call_arguments_builder.hpp"
#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/register_values_builder.hpp"

#include <concepts>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/elements/handle.hpp>
#include <utility>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/columns_builder.hpp>
#include <yggdrasil/formalism/free_relations.hpp>
#include <yggdrasil/formalism/relation_builder.hpp>
#include <yggdrasil/formalism/row_builder.hpp>

namespace runir::kr::dl::semantics
{
/// The builders and scratch records of every record the denotation repository holds (denotations, their words,
/// register values, call arguments): read through it, each child through the repository that holds its kind (an
/// object through the planning repository, the rest through this one).
using RecordStorage = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, boost::mp11::mp_push_front<DenotationRecordTypes, DenotationRepository>>;

/// The builders and scratch records of the query relations interned with denotations: free relations'.
using RelationStorage = ygg::FreeRelationStorage;

/// Whether a builder of T (over the denotation repository) is initialized from a Value alone: a boolean's or a
/// numerical denotation's value.
template<typename T, typename Value>
concept InitializesFrom = requires(urd::builder<T, DenotationRepository>& builder, Value value) { builder.initialize(value); };

class Builder
{
private:
    RecordStorage m_records;
    RelationStorage m_relations;
    EvaluationWorkspace m_workspace;

public:
    /// Shared evaluation scratch, retaining mutable buffers across states.
    auto& get_workspace() noexcept { return m_workspace; }

    /// A cleared builder of a record the denotation repository holds.
    template<typename T>
        requires boost::mp11::mp_contains<DenotationRecordTypes, T>::value
    [[nodiscard]] urd::handle<urd::builder<T, DenotationRepository>> get_builder()
    { return m_records.template checkout<T>(); }

    /// A builder of a boolean or numerical denotation holding `value`.
    template<typename T, typename Value>
        requires boost::mp11::mp_contains<DenotationTypes, T>::value && InitializesFrom<T, Value>
    [[nodiscard]] urd::handle<urd::builder<T, DenotationRepository>> get_builder(Value value)
    {
        auto builder = m_records.template checkout<T>();
        builder->initialize(value);
        return builder;
    }

    /// An empty builder of a concept's or role's denotation over num_objects objects, its words drawn from this
    /// storage.
    template<typename T>
        requires boost::mp11::mp_contains<DenotationTypes, T>::value
                 && requires(urd::builder<T, DenotationRepository>& built, RecordStorage& records) { semantics::initialize(built, ygg::uint_t { 0 }, records); }
    [[nodiscard]] urd::handle<urd::builder<T, DenotationRepository>> get_builder(ygg::uint_t num_objects)
    {
        auto builder = m_records.template checkout<T>();
        semantics::initialize(*builder, num_objects, m_records);
        return builder;
    }

    /// The storage the denotation repository's records are interned through (repository.insert(builder, records())).
    [[nodiscard]] RecordStorage& records() noexcept { return m_records; }

    /// The storage of the query relations' builders and scratch records.
    [[nodiscard]] RelationStorage& relations() noexcept { return m_relations; }

    /// A cleared scratch record of a record the denotation repository holds.
    template<typename T>
        requires boost::mp11::mp_contains<DenotationRecordTypes, T>::value
    [[nodiscard]] urd::handle<urd::data<T>> scratch()
    { return m_records.template scratch<T>(); }
};

}  // namespace runir::kr::dl::semantics

#endif
