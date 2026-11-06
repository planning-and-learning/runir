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
#include <urd/repr/slots/slots.hpp>
#include <urd/store.hpp>
#include <yggdrasil/database/semantics/relation_pool.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::dl::semantics
{

class Builder
{
private:
    template<typename T>
    using DenotationPool = urd::Store<urd::slots<urd::builder<T>>>;
    using DenotationBuilderStorage = boost::mp11::mp_rename<boost::mp11::mp_transform<DenotationPool, DenotationTypes>, std::tuple>;
    using DenotationDataStorage = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, DenotationRecordTypes>;

    DenotationBuilderStorage m_builders;
    DenotationDataStorage m_data;
    ygg::database::RelationPool m_relation_builders;
    EvaluationWorkspace m_workspace;

public:
    /// Shared evaluation scratch, retaining mutable buffers across states.
    auto& get_workspace() noexcept { return m_workspace; }

    template<typename T>
        requires(boost::mp11::mp_contains<DenotationTypes, T>::value || std::same_as<T, ygg::database::Relation>)
    [[nodiscard]] auto get_builder()
    {
        if constexpr (std::same_as<T, ygg::database::Relation>)
            return m_relation_builders.get_or_allocate({});
        else
            return std::get<urd::Store<urd::slots<urd::builder<T>>>>(m_builders).acquire_unique();
    }

    template<typename T, typename... Args>
        requires(sizeof...(Args) > 0
                 && ((boost::mp11::mp_contains<DenotationTypes, T>::value && requires(urd::builder<T>& builder, Args&&... args) { builder.initialize(std::forward<Args>(args)...); })
                     || (std::same_as<T, ygg::database::Relation>
                         && requires(ygg::database::RelationPool& pool, Args&&... args) { pool.get_or_allocate(std::forward<Args>(args)...); })))
    [[nodiscard]] auto get_builder(Args&&... args)
    {
        if constexpr (std::same_as<T, ygg::database::Relation>)
            return m_relation_builders.get_or_allocate(std::forward<Args>(args)...);
        else
        {
            auto builder = std::get<urd::Store<urd::slots<urd::builder<T>>>>(m_builders).acquire_unique();
            builder->initialize(std::forward<Args>(args)...);
            return builder;
        }
    }

    template<typename T>
        requires boost::mp11::mp_contains<DenotationRecordTypes, T>::value
    [[nodiscard]] auto get_data()
    { return m_data.template get_builder<T>(); }

    template<typename T>
        requires boost::mp11::mp_contains<DenotationRecordTypes, T>::value
    [[nodiscard]] auto checkout()
    { return m_data.template checkout<T>(); }
};

template<typename T>
    requires boost::mp11::mp_contains<DenotationRecordTypes, T>::value
[[nodiscard]] auto checkout(Builder& builder)
{ return builder.template checkout<T>(); }

}  // namespace runir::kr::dl::semantics

#endif
