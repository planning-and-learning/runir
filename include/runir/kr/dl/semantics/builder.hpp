#ifndef RUNIR_KR_DL_SEMANTICS_BUILDER_HPP_
#define RUNIR_KR_DL_SEMANTICS_BUILDER_HPP_

#include "runir/kr/dl/semantics/call_arguments_data.hpp"
#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/register_values_data.hpp"

#include <concepts>
#include <tuple>
#include <utility>
#include <yggdrasil/containers/unique_object_pool.hpp>
#include <yggdrasil/database/relation_pool.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::dl::semantics
{

class Builder
{
private:
    using DenotationBuilderStorage = std::tuple<ygg::UniqueObjectPool<ygg::Builder<Denotation<BooleanTag>>>,
                                                ygg::UniqueObjectPool<ygg::Builder<Denotation<NumericalTag>>>,
                                                ygg::UniqueObjectPool<ygg::Builder<Denotation<ConceptTag>>>,
                                                ygg::UniqueObjectPool<ygg::Builder<Denotation<RoleTag>>>>;
    using DenotationDataStorage = ygg::formalism::
        BuilderStorage<Denotation<BooleanTag>, Denotation<NumericalTag>, Denotation<ConceptTag>, Denotation<RoleTag>, RegisterValues, CallArguments>;

    DenotationBuilderStorage m_builders;
    DenotationDataStorage m_data;
    ygg::database::RelationPool<> m_relation_builders;
    EvaluationWorkspace m_workspace;

public:
    /// Shared evaluation scratch, retaining mutable buffers across states.
    auto& get_workspace() noexcept { return m_workspace; }

    template<typename T>
    [[nodiscard]] auto get_builder()
    {
        if constexpr (std::same_as<T, ygg::database::Relation<>>)
            return m_relation_builders.get_or_allocate({});
        else
            return std::get<ygg::UniqueObjectPool<ygg::Builder<T>>>(m_builders).get_or_allocate();
    }

    template<typename T, typename... Args>
    [[nodiscard]] auto get_builder(Args&&... args)
    {
        if constexpr (std::same_as<T, ygg::database::Relation<>>)
            return m_relation_builders.get_or_allocate(std::forward<Args>(args)...);
        else
            return std::get<ygg::UniqueObjectPool<ygg::Builder<T>>>(m_builders).get_or_allocate(std::forward<Args>(args)...);
    }

    template<typename T>
    [[nodiscard]] auto get_data()
    {
        return m_data.template get_builder<T>();
    }

    template<typename T>
    [[nodiscard]] auto checkout()
    {
        return m_data.template checkout<T>();
    }
};

template<typename T>
[[nodiscard]] auto checkout(Builder& builder)
{
    return builder.template checkout<T>();
}

}  // namespace runir::kr::dl::semantics

#endif
