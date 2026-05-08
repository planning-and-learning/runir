#ifndef RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_
#define RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_

#include "runir/semantics/denotation_builder.hpp"
#include "runir/semantics/denotation_data.hpp"
#include "runir/semantics/denotation_index.hpp"

#include <cassert>
#include <concepts>
#include <tuple>
#include <tyr/common/dynamic_bitset.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/object_index.hpp>

namespace tyr
{

template<runir::CategoryTag Category, typename C>
class View<Index<runir::semantics::Denotation<Category>>, C>
{
private:
    const C* m_context;
    Index<runir::semantics::Denotation<Category>> m_handle;

    auto get_vector() const noexcept
        requires(std::same_as<Category, runir::ConceptTag> || std::same_as<Category, runir::RoleTag>)
    {
        return get_denotation_vector_repository(*m_context)[get_data().vec_index];
    }

public:
    View(Index<runir::semantics::Denotation<Category>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_denotation_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto get() const noexcept
        requires(std::same_as<Category, runir::BooleanTag> || std::same_as<Category, runir::NumericalTag>)
    {
        return get_data().get_data();
    }

    auto get() const noexcept
        requires(std::same_as<Category, runir::ConceptTag>)
    {
        using Layout = Builder<runir::semantics::Denotation<runir::ConceptTag>>;
        using Bitset = BitsetSpan<const runir::uint_t>;

        const auto vector = get_vector();
        const auto& data = get_data();

        assert(vector.size() == Layout::num_blocks(data.num_objects));

        return Bitset(vector.data(), Layout::num_bits(data.num_objects));
    }

    auto get(Index<formalism::Object> object) const noexcept
        requires(std::same_as<Category, runir::RoleTag>)
    {
        using Layout = Builder<runir::semantics::Denotation<runir::RoleTag>>;
        using Bitset = BitsetSpan<const runir::uint_t>;

        const auto vector = get_vector();
        const auto& data = get_data();

        assert(vector.size() == Layout::num_blocks(data.num_objects));
        assert(uint_t(object) < data.num_objects);

        return Bitset(vector.data() + Layout::row_block_offset(object, data.num_objects), data.num_objects);
    }

    auto identifying_members() const noexcept { return std::tie(get_data()); }
};

}

#endif
