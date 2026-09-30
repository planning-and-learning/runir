#ifndef RUNIR_KR_PS_EXT_DETAIL_TRANSIENT_VALUES_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_TRANSIENT_VALUES_HPP_

#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/register_values_data.hpp"

#include <concepts>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace runir::kr::ps::ext::detail
{
namespace transient_dl = runir::kr::dl;
namespace transient_semantics = runir::kr::dl::semantics;

using OwnedRegisterValues = ygg::Data<transient_semantics::RegisterValues>;

/// Borrows register data and the task's formalism repository.
class RegisterValuesRef
{
    const OwnedRegisterValues* m_data;
    const tyr::formalism::planning::Repository* m_repository;

public:
    RegisterValuesRef(const OwnedRegisterValues& data, const tyr::formalism::planning::Repository& repository) noexcept :
        m_data(&data),
        m_repository(&repository)
    {
    }

    const auto& get_data() const noexcept { return *m_data; }
    const auto& get_context() const noexcept { return *m_repository; }
    auto get_concept_values() const noexcept { return ygg::make_view(m_data->concept_values, *m_repository); }
    auto get_role_values() const noexcept { return ygg::make_view(m_data->role_values, *m_repository); }

    template<transient_dl::ConceptOrRoleTag Category>
    auto get() const noexcept
    {
        if constexpr (std::same_as<Category, transient_dl::ConceptTag>)
            return get_concept_values();
        else
            return get_role_values();
    }

    template<transient_dl::ConceptOrRoleTag Category>
    auto at(transient_dl::RegisterIdentifier<Category> identifier) const
    {
        return get<Category>().at(static_cast<size_t>(ygg::uint_t(identifier)));
    }
};

template<transient_dl::CategoryTag Category>
struct OwnedDenotation : ygg::Builder<transient_semantics::Denotation<Category>>
{
    using Base = ygg::Builder<transient_semantics::Denotation<Category>>;
    using Base::Base;

    auto identifying_members() const noexcept
    {
        if constexpr (transient_dl::ConceptOrRoleTag<Category>)
            return std::tie(this->num_objects, this->blocks);
        else
            return std::tie(this->value);
    }
};

struct OwnedCallArguments
{
private:
    template<transient_dl::CategoryTag Category>
    struct Buffer
    {
        std::vector<OwnedDenotation<Category>> values;
        size_t size = 0;
    };
    std::tuple<Buffer<transient_dl::ConceptTag>, Buffer<transient_dl::RoleTag>, Buffer<transient_dl::BooleanTag>, Buffer<transient_dl::NumericalTag>> m_buffers;

public:
    // Keep inactive denotation slots so their bitset allocations survive pool reuse.
    void initialize() noexcept
    {
        std::apply([](auto&... buffers) { ((buffers.size = 0), ...); }, m_buffers);
    }
    void release_owners() noexcept { initialize(); }
    const auto& get_data() const noexcept { return *this; }
    template<transient_dl::CategoryTag Category>
    auto get() const noexcept
    {
        const auto& buffer = std::get<Buffer<Category>>(m_buffers);
        return std::span<const OwnedDenotation<Category>>(buffer.values.data(), buffer.size);
    }

    auto identifying_members() const noexcept
    {
        return std::make_tuple(get<transient_dl::ConceptTag>(),
                               get<transient_dl::RoleTag>(),
                               get<transient_dl::BooleanTag>(),
                               get<transient_dl::NumericalTag>());
    }

    template<transient_dl::CategoryTag Category>
    void append(const auto& value)
    {
        auto& buffer = std::get<Buffer<Category>>(m_buffers);
        if (buffer.size == buffer.values.size())
            buffer.values.emplace_back();
        auto& target = buffer.values[buffer.size];
        if constexpr (std::same_as<Category, transient_dl::ConceptTag>)
        {
            target.initialize(static_cast<ygg::uint_t>(value.get().size()));
            target.get().copy_from(value.get());
        }
        else if constexpr (std::same_as<Category, transient_dl::RoleTag>)
        {
            target.initialize(value.get_num_objects());
            for (ygg::uint_t object = 0; object < target.num_objects; ++object)
                target.get(object).copy_from(value.get(object));
        }
        else
            target.initialize(value.get());
        ++buffer.size;
    }

    template<transient_dl::CategoryTag Category>
    const auto& at(transient_dl::ArgumentIdentifier<Category> identifier) const
    {
        const auto index = static_cast<size_t>(ygg::uint_t(identifier));
        const auto values = get<Category>();
        if (index >= values.size())
            throw std::out_of_range("Transient call argument index out of range.");
        return values[index];
    }
};

/// Contexts copy this reference, keeping argument denotations in their owning frame.
class CallArgumentsRef
{
    const OwnedCallArguments* m_data;

public:
    explicit CallArgumentsRef(const OwnedCallArguments& data) noexcept : m_data(&data) {}

    const auto& get_data() const noexcept { return *m_data; }

    template<transient_dl::CategoryTag Category>
    auto get() const noexcept
    {
        return m_data->get<Category>();
    }

    template<transient_dl::CategoryTag Category>
    const auto& at(transient_dl::ArgumentIdentifier<Category> identifier) const
    {
        return m_data->at(identifier);
    }
};

template<transient_dl::CategoryTag Category>
void append_call_argument(OwnedCallArguments& arguments, const auto& value)
{
    arguments.append<Category>(value);
}

inline transient_semantics::CallArgumentsView
materialize_call_arguments(const OwnedCallArguments& arguments, transient_semantics::Builder& builder, transient_semantics::DenotationRepository& repository)
{
    auto data = transient_semantics::checkout<transient_semantics::CallArguments>(builder);
    const auto copy = [&]<transient_dl::CategoryTag Category>(auto& indices)
    {
        auto denotation = transient_semantics::checkout<transient_semantics::Denotation<Category>>(builder);
        for (const auto& value : arguments.get<Category>())
        {
            transient_semantics::make_data(value, *denotation);
            if constexpr (transient_dl::ConceptOrRoleTag<Category>)
                denotation->vec_index = repository.get_vector_repository().insert(value.blocks);
            indices.push_back(transient_semantics::get_or_create(repository, *denotation).first.get_index());
        }
    };
    copy.template operator()<transient_dl::ConceptTag>(data->concept_arguments);
    copy.template operator()<transient_dl::RoleTag>(data->role_arguments);
    copy.template operator()<transient_dl::BooleanTag>(data->boolean_arguments);
    copy.template operator()<transient_dl::NumericalTag>(data->numerical_arguments);
    return transient_semantics::get_or_create(repository, *data).first;
}

inline transient_semantics::RegisterValuesView
materialize_register_values(const OwnedRegisterValues& registers, transient_semantics::Builder& builder, transient_semantics::DenotationRepository& repository)
{
    auto data = transient_semantics::checkout<transient_semantics::RegisterValues>(builder);
    data->concept_values = registers.concept_values;
    data->role_values = registers.role_values;
    return transient_semantics::get_or_create(repository, *data).first;
}

}  // namespace runir::kr::ps::ext::detail

#endif
