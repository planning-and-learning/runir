#ifndef RUNIR_SEMANTICS_BUILDER_HPP_
#define RUNIR_SEMANTICS_BUILDER_HPP_

#include "runir/knowledge_representation/dl/semantics/denotation_builder.hpp"
#include "runir/knowledge_representation/dl/semantics/denotation_index.hpp"

#include <tuple>
#include <tyr/common/unique_object_pool.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

template<typename T>
class BasicBuilder
{
private:
    using Pooled = tyr::Builder<T>;
    using Pool = tyr::UniqueObjectPool<Pooled>;

    Pool m_pool;

public:
    BasicBuilder() = default;

    [[nodiscard]] auto get_builder() { return m_pool.get_or_allocate(); }

    template<typename... Args>
    [[nodiscard]] auto get_builder(Args&&... args)
    {
        return m_pool.get_or_allocate(std::forward<Args>(args)...);
    }
};

class Builder
{
private:
    using BuilderStorage = std::tuple<BasicBuilder<Denotation<BooleanTag>>,
                                      BasicBuilder<Denotation<NumericalTag>>,
                                      BasicBuilder<Denotation<ConceptTag>>,
                                      BasicBuilder<Denotation<RoleTag>>>;

    BuilderStorage m_builders;

public:
    Builder() = default;

    template<typename T>
    [[nodiscard]] auto get_builder()
    {
        return std::get<BasicBuilder<T>>(m_builders).get_builder();
    }

    template<typename T, typename... Args>
    [[nodiscard]] auto get_builder(Args&&... args)
    {
        return std::get<BasicBuilder<T>>(m_builders).get_builder(std::forward<Args>(args)...);
    }
};

}

#endif
