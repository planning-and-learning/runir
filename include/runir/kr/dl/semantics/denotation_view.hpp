#ifndef RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_
#define RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_

#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/denotation_index.hpp"

#include <cassert>
#include <concepts>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/containers/dynamic_bitset.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::semantics
{

template<runir::kr::dl::CategoryTag Category>
struct DenotationElementType;

template<>
struct DenotationElementType<runir::kr::dl::BooleanTag>
{
    using Type = bool;
};

template<>
struct DenotationElementType<runir::kr::dl::NumericalTag>
{
    using Type = ygg::uint_t;
};

template<>
struct DenotationElementType<runir::kr::dl::ConceptTag>
{
    using Type = tyr::formalism::planning::ObjectView;
};

template<>
struct DenotationElementType<runir::kr::dl::RoleTag>
{
    using Type = std::pair<tyr::formalism::planning::ObjectView, tyr::formalism::planning::ObjectView>;
};

template<runir::kr::dl::CategoryTag Category>
using DenotationElementView = typename DenotationElementType<Category>::Type;

template<runir::kr::dl::CategoryTag Category>
using DenotationElementViewList = std::vector<DenotationElementView<Category>>;

}  // namespace runir::kr::dl::semantics

namespace ygg
{

template<runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::semantics::Denotation<Category>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::semantics::Denotation<Category>> m_handle;

    static constexpr auto npos = BitsetSpan<const ygg::uint_t>::npos;
    using BitIterator = SetBitIndices<ygg::uint_t>::Iterator;

    auto get_vector() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return get_denotation_vector_repository(*m_context)[get_data().vec_index];
    }

public:
    class ConceptIterator
    {
    private:
        Index<runir::kr::dl::semantics::Denotation<Category>> m_handle;
        const C* m_context = nullptr;
        BitIterator m_object;

    public:
        ConceptIterator() = default;
        ConceptIterator(const View& view, bool begin) noexcept :
            m_handle(view.get_handle()),
            m_context(&view.get_context()),
            m_object(begin ? set_bit_indices(view.get()).begin() : BitIterator(view.get(), npos))
        {
        }

        auto operator*() const noexcept -> runir::kr::dl::semantics::DenotationElementView<runir::kr::dl::ConceptTag>
        {
            return make_view(Index<::tyr::formalism::Object>(static_cast<ygg::uint_t>(*m_object)), m_context->get_formalism_repository());
        }

        ConceptIterator& operator++() noexcept
        {
            ++m_object;
            return *this;
        }

        friend bool operator==(const ConceptIterator& lhs, const ConceptIterator& rhs) noexcept
        {
            return lhs.m_handle == rhs.m_handle && lhs.m_context == rhs.m_context && lhs.m_object == rhs.m_object;
        }

        friend bool operator!=(const ConceptIterator& lhs, const ConceptIterator& rhs) noexcept { return !(lhs == rhs); }
    };

    class RoleIterator
    {
    private:
        Index<runir::kr::dl::semantics::Denotation<Category>> m_handle;
        const C* m_context = nullptr;
        size_t m_source = npos;
        BitIterator m_target;

        void advance_to_next_nonempty_row() noexcept
        {
            const auto view = View(m_handle, *m_context);
            const auto num_objects = view.get_data().num_objects;
            while (m_source < num_objects)
            {
                const auto row = view.get(Index<::tyr::formalism::Object>(static_cast<ygg::uint_t>(m_source)));
                const auto targets = set_bit_indices(row);
                m_target = targets.begin();
                if (m_target != targets.end())
                    return;
                ++m_source;
            }

            m_source = npos;
            m_target = {};
        }

    public:
        RoleIterator() = default;
        RoleIterator(const View& view, size_t source) noexcept :
            m_handle(view.get_handle()),
            m_context(&view.get_context()),
            m_source(source)
        {
            if (m_source != npos)
                advance_to_next_nonempty_row();
        }

        auto operator*() const noexcept -> runir::kr::dl::semantics::DenotationElementView<runir::kr::dl::RoleTag>
        {
            return std::pair(make_view(Index<::tyr::formalism::Object>(static_cast<ygg::uint_t>(m_source)), m_context->get_formalism_repository()),
                             make_view(Index<::tyr::formalism::Object>(static_cast<ygg::uint_t>(*m_target)), m_context->get_formalism_repository()));
        }

        RoleIterator& operator++() noexcept
        {
            ++m_target;
            if (m_target == std::default_sentinel)
            {
                ++m_source;
                advance_to_next_nonempty_row();
            }
            return *this;
        }

        friend bool operator==(const RoleIterator& lhs, const RoleIterator& rhs) noexcept
        {
            return lhs.m_handle == rhs.m_handle && lhs.m_context == rhs.m_context && lhs.m_source == rhs.m_source && lhs.m_target == rhs.m_target;
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
        using Bitset = BitsetSpan<const ygg::uint_t>;

        const auto vector = get_vector();
        const auto& data = get_data();

        assert(vector.size() == Layout::num_blocks(data.num_objects));

        return Bitset(vector.data(), Layout::num_bits(data.num_objects));
    }

    auto get(Index<::tyr::formalism::Object> object) const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        using Layout = Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>;
        using Bitset = BitsetSpan<const ygg::uint_t>;

        const auto vector = get_vector();
        const auto& data = get_data();

        assert(vector.size() == Layout::num_blocks(data.num_objects));
        assert(ygg::uint_t(object) < data.num_objects);

        return Bitset(vector.data() + Layout::row_block_offset(object, data.num_objects), data.num_objects);
    }

    auto get(ygg::uint_t object) const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return get(Index<::tyr::formalism::Object>(object));
    }

    auto get_num_objects() const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return get_data().num_objects;
    }

    /// Includes the zero padding at the end of every role row.
    auto storage_bits() const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        using Bitset = BitsetSpan<const ygg::uint_t>;
        const auto vector = get_vector();
        return Bitset(vector.data(), vector.size() * Bitset::Digits);
    }

    bool any() const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return storage_bits().any();
    }

    auto count() const noexcept -> size_t
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return storage_bits().count();
    }

    auto begin() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        return ConceptIterator(*this, true);
    }

    auto end() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        return ConceptIterator(*this, false);
    }

    auto begin() const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return RoleIterator(*this, 0);
    }

    auto end() const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return RoleIterator(*this, npos);
    }

    auto identifying_members() const noexcept { return std::make_tuple(m_handle, get_denotation_repository(*m_context).get_index()); }
};

}

#endif
