#ifndef RUNIR_SEMANTICS_BUILDER_HPP_
#define RUNIR_SEMANTICS_BUILDER_HPP_

#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/denotation_index.hpp"

#include <tuple>
#include <utility>
#include <yggdrasil/containers/unique_object_pool.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::dl::semantics
{

template<typename T>
class BasicBuilder
{
private:
    using Pooled = ygg::Builder<T>;
    using Pool = ygg::UniqueObjectPool<Pooled>;

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
    using DenotationBuilderStorage = std::tuple<BasicBuilder<Denotation<BooleanTag>>,
                                                BasicBuilder<Denotation<NumericalTag>>,
                                                BasicBuilder<Denotation<ConceptTag>>,
                                                BasicBuilder<Denotation<RoleTag>>>;
    using DenotationDataStorage = ygg::formalism::BuilderStorage<Denotation<BooleanTag>, Denotation<NumericalTag>, Denotation<ConceptTag>, Denotation<RoleTag>>;

    DenotationBuilderStorage m_builders;
    DenotationDataStorage m_data;

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

    template<typename T>
    [[nodiscard]] auto get_data()
    {
        return m_data.template get_builder<T>();
    }
};

inline ygg::Data<Denotation<BooleanTag>>& make_data(const ygg::Builder<Denotation<BooleanTag>>& builder, ygg::Data<Denotation<BooleanTag>>& data) noexcept
{
    data = ygg::Data<Denotation<BooleanTag>>(builder.value);
    return data;
}

inline ygg::Data<Denotation<NumericalTag>>& make_data(const ygg::Builder<Denotation<NumericalTag>>& builder, ygg::Data<Denotation<NumericalTag>>& data) noexcept
{
    data = ygg::Data<Denotation<NumericalTag>>(builder.value);
    return data;
}

inline ygg::Data<Denotation<ConceptTag>>& make_data(const ygg::Builder<Denotation<ConceptTag>>& builder, ygg::Data<Denotation<ConceptTag>>& data) noexcept
{
    data.clear();
    data.num_objects = builder.num_objects;
    return data;
}

inline ygg::Data<Denotation<RoleTag>>& make_data(const ygg::Builder<Denotation<RoleTag>>& builder, ygg::Data<Denotation<RoleTag>>& data) noexcept
{
    data.clear();
    data.num_objects = builder.num_objects;
    return data;
}

}

#endif
