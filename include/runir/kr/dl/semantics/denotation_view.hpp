#ifndef RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_
#define RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_

#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/denotation_index.hpp"

#include <cassert>
#include <concepts>
#include <tuple>
#include <tyr/common/dynamic_bitset.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/object_view.hpp>
#include <utility>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::semantics::Denotation<Category>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::semantics::Denotation<Category>> m_handle;

    static constexpr auto npos = BitsetSpan<const runir::uint_t>::npos;

    auto get_vector() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return get_denotation_vector_repository(*m_context)[get_data().vec_index];
    }

public:
    class ConceptIterator
    {
    private:
        const View* m_view = nullptr;
        size_t m_object = npos;

    public:
        ConceptIterator() = default;
        ConceptIterator(const View& view, size_t object) noexcept : m_view(&view), m_object(object) {}

        auto operator*() const noexcept { return make_view(Index<formalism::Object>(static_cast<uint_t>(m_object)), m_view->get_context()); }

        ConceptIterator& operator++() noexcept
        {
            m_object = m_view->get().find_next(m_object);
            return *this;
        }

        friend bool operator==(const ConceptIterator& lhs, const ConceptIterator& rhs) noexcept
        {
            return lhs.m_view == rhs.m_view && lhs.m_object == rhs.m_object;
        }

        friend bool operator!=(const ConceptIterator& lhs, const ConceptIterator& rhs) noexcept { return !(lhs == rhs); }
    };

    class RoleIterator
    {
    private:
        const View* m_view = nullptr;
        size_t m_source = npos;
        size_t m_target = npos;

        void advance_to_next_nonempty_row() noexcept
        {
            const auto num_objects = m_view->get_data().num_objects;
            while (m_source < num_objects)
            {
                const auto row = m_view->get(Index<formalism::Object>(static_cast<uint_t>(m_source)));
                m_target = row.find_first();
                if (m_target != npos)
                    return;
                ++m_source;
            }

            m_source = npos;
            m_target = npos;
        }

    public:
        RoleIterator() = default;
        RoleIterator(const View& view, size_t source, size_t target) noexcept : m_view(&view), m_source(source), m_target(target)
        {
            if (m_source != npos && m_target == npos)
                advance_to_next_nonempty_row();
        }

        auto operator*() const noexcept
        {
            return std::pair(make_view(Index<formalism::Object>(static_cast<uint_t>(m_source)), m_view->get_context()),
                             make_view(Index<formalism::Object>(static_cast<uint_t>(m_target)), m_view->get_context()));
        }

        RoleIterator& operator++() noexcept
        {
            const auto row = m_view->get(Index<formalism::Object>(static_cast<uint_t>(m_source)));
            m_target = row.find_next(m_target);
            if (m_target == npos)
            {
                ++m_source;
                advance_to_next_nonempty_row();
            }
            return *this;
        }

        friend bool operator==(const RoleIterator& lhs, const RoleIterator& rhs) noexcept
        {
            return lhs.m_view == rhs.m_view && lhs.m_source == rhs.m_source && lhs.m_target == rhs.m_target;
        }

        friend bool operator!=(const RoleIterator& lhs, const RoleIterator& rhs) noexcept { return !(lhs == rhs); }
    };

    View(Index<runir::kr::dl::semantics::Denotation<Category>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_denotation_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto get() const noexcept
        requires(std::same_as<Category, runir::kr::dl::BooleanTag> || std::same_as<Category, runir::kr::dl::NumericalTag>)
    {
        return get_data().get_data();
    }

    auto get() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        using Layout = Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>;
        using Bitset = BitsetSpan<const runir::uint_t>;

        const auto vector = get_vector();
        const auto& data = get_data();

        assert(vector.size() == Layout::num_blocks(data.num_objects));

        return Bitset(vector.data(), Layout::num_bits(data.num_objects));
    }

    auto get(Index<formalism::Object> object) const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        using Layout = Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>;
        using Bitset = BitsetSpan<const runir::uint_t>;

        const auto vector = get_vector();
        const auto& data = get_data();

        assert(vector.size() == Layout::num_blocks(data.num_objects));
        assert(uint_t(object) < data.num_objects);

        return Bitset(vector.data() + Layout::row_block_offset(object, data.num_objects), data.num_objects);
    }

    auto begin() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        return ConceptIterator(*this, get().find_first());
    }

    auto end() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        return ConceptIterator(*this, npos);
    }

    auto begin() const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return RoleIterator(*this, 0, npos);
    }

    auto end() const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return RoleIterator(*this, npos, npos);
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
