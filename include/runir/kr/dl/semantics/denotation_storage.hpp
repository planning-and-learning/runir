#ifndef RUNIR_SEMANTICS_DENOTATION_STORAGE_HPP_
#define RUNIR_SEMANTICS_DENOTATION_STORAGE_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <ranges>
#include <span>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <urd/bits.hpp>
#include <urd/elements/children.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/elements/data.hpp>
#include <urd/elements/flat.hpp>
#include <urd/elements/index.hpp>
#include <urd/elements/view.hpp>
#include <urd/kinds.hpp>
#include <urd/repr/pools/pools.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/tags.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>

// A concept's or role's denotation, as tyr's state storages are: its record holds what it denotes (values, its words:
// DenotationWords), its builder a bitset of its own (DenotationBitset: its built form), and its repository packs it into
// a pool of its own (DenotationPool), whose lookup hands out a reader (StoredDenotation) that reads its values in place
// (DenotationBits), resolved through a view's context as the objects they denote (ResolvedDenotationBits).

namespace runir::kr::dl::semantics
{
namespace detail
{
/// Where a concept's bits lie in its words: a bit per object, the trailing bits zero.
struct ConceptLayout
{
    static constexpr auto num_bits(ygg::uint_t num_objects) noexcept -> size_t { return num_objects; }

    static constexpr auto num_blocks(ygg::uint_t num_objects) noexcept -> size_t { return urd::bitset_span<ygg::uint_t>::num_blocks(num_bits(num_objects)); }
};

/// Where a role's bits lie in its words: a row of whole blocks per source object, each row's trailing bits zero.
struct RoleLayout
{
    static constexpr auto num_bits(ygg::uint_t num_objects) noexcept -> size_t { return static_cast<size_t>(num_objects) * static_cast<size_t>(num_objects); }

    static constexpr auto num_row_blocks(ygg::uint_t num_objects) noexcept -> size_t { return urd::bitset_span<ygg::uint_t>::num_blocks(num_objects); }

    static constexpr auto num_blocks(ygg::uint_t num_objects) noexcept -> size_t { return static_cast<size_t>(num_objects) * num_row_blocks(num_objects); }

    static constexpr auto row_block_offset(urd::index<tyr::formalism::Object> object, ygg::uint_t num_objects) noexcept -> size_t
    { return static_cast<size_t>(static_cast<ygg::uint_t>(object.value)) * num_row_blocks(num_objects); }
};

/// The layout of a denotation of Category.
template<ConceptOrRoleTag Category>
using Layout = std::conditional_t<std::same_as<Category, ConceptTag>, ConceptLayout, RoleLayout>;

/// The object a set bit names: its position.
struct ObjectOfBit
{
    [[nodiscard]] urd::index<tyr::formalism::Object> operator()(std::size_t bit) const noexcept
    { return urd::index<tyr::formalism::Object>(static_cast<ygg::uint_t>(bit)); }
};

/// The objects whose bits are set (a concept's bits, or a role's row): a borrowed range, holding only the bits.
using ObjectIndices = urd::transformed_range<urd::set_bit_indices_view<ygg::uint_t>, ObjectOfBit>;

inline ObjectIndices object_indices(urd::bitset_span<const ygg::uint_t> bits) noexcept { return urd::transform(urd::set_bit_indices(bits), ObjectOfBit {}); }

/// The (source, target) pairs whose bits are set in a role's bits, a row of whole blocks per source.
inline auto object_pair_indices(urd::bitset_span<const ygg::uint_t> bits, ygg::uint_t num_objects) noexcept
{
    using Bitset = urd::bitset_span<const ygg::uint_t>;
    return urd::transform(urd::set_bit_indices(bits),
                          static_cast<std::size_t>(Bitset::num_blocks(num_objects) * Bitset::Digits),
                          [](std::size_t row_bits, std::size_t bit)
                          {
                              return std::pair(urd::index<tyr::formalism::Object>(static_cast<ygg::uint_t>(bit / row_bits)),
                                               urd::index<tyr::formalism::Object>(static_cast<ygg::uint_t>(bit % row_bits)));
                          });
}
}  // namespace detail

/// A concept's or role's values read in place (a stored record's, or a builder's): its words over its num_objects
/// objects (detail::ConceptLayout, detail::RoleLayout). It iterates as what it denotes, as indices: a concept's objects,
/// a role's (source, target) pairs; its hot readers read its words. Borrows the words, which must outlive it.
template<ConceptOrRoleTag Category>
class DenotationBits : public std::ranges::view_interface<DenotationBits<Category>>
{
    const ygg::uint_t* m_words = nullptr;
    ygg::uint_t m_num_objects = 0;

public:
    DenotationBits() noexcept = default;

    DenotationBits(const ygg::uint_t* words, ygg::uint_t num_objects) noexcept : m_words(words), m_num_objects(num_objects) {}

    [[nodiscard]] ygg::uint_t num_objects() const noexcept { return m_num_objects; }

    [[nodiscard]] std::span<const ygg::uint_t> words() const noexcept { return { m_words, detail::Layout<Category>::num_blocks(m_num_objects) }; }

    /// Its words as bits, trailing padding included.
    [[nodiscard]] urd::bitset_span<const ygg::uint_t> storage_bits() const noexcept
    {
        const auto all = words();
        return urd::bitset_span<const ygg::uint_t>(all.data(), all.size() * urd::bitset_span<const ygg::uint_t>::Digits);
    }

    /// What it denotes, as indices: a concept's objects, a role's (source, target) pairs.
    [[nodiscard]] auto elements() const noexcept
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            return detail::object_indices(storage_bits());
        else
            return detail::object_pair_indices(storage_bits(), m_num_objects);
    }

    [[nodiscard]] auto begin() const noexcept { return elements().begin(); }

    [[nodiscard]] auto end() const noexcept { return elements().end(); }
};

/// A denotation's values resolved through a view's context C (urd::resolution): it iterates as the views of what it
/// denotes (a concept's objects, a role's pairs of them), and keeps the values it reads (bits()) for the hot readers.
template<ConceptOrRoleTag Category, typename C>
class ResolvedDenotationBits : public std::ranges::view_interface<ResolvedDenotationBits<Category, C>>
{
    DenotationBits<Category> m_bits;
    const C* m_context = nullptr;

public:
    ResolvedDenotationBits() noexcept = default;

    ResolvedDenotationBits(DenotationBits<Category> bits, const C* context) noexcept : m_bits(bits), m_context(context) {}

    [[nodiscard]] DenotationBits<Category> bits() const noexcept { return m_bits; }

    [[nodiscard]] auto begin() const { return urd::transform(m_bits, urd::resolver<C> { m_context }).begin(); }

    [[nodiscard]] auto end() const { return urd::transform(m_bits, urd::resolver<C> { m_context }).end(); }
};

/// A concept's or role's values being built: its words, its own (set and flipped in place, its buffer kept where it is
/// cleared or initialized again), over num_objects objects (its builder's).
template<ConceptOrRoleTag Category>
struct DenotationBitset
{
    ygg::uint_t num_objects = 0;
    std::vector<ygg::uint_t> words;

    /// Empty over num_objects objects: zero words.
    void initialize(ygg::uint_t num_objects_)
    {
        num_objects = num_objects_;
        words.assign(detail::Layout<Category>::num_blocks(num_objects_), ygg::uint_t { 0 });
    }

    /// A copy of `source` (a no-op where it reads these words).
    void assign(DenotationBits<Category> source)
    {
        num_objects = source.num_objects();
        const auto source_words = source.words();
        if (source_words.data() != words.data())
            words.assign(source_words.begin(), source_words.end());
    }

    void clear() noexcept
    {
        num_objects = 0;
        words.clear();
    }

    [[nodiscard]] DenotationBits<Category> bits() const noexcept { return DenotationBits<Category>(words.data(), num_objects); }

    static constexpr auto identifying_members() noexcept { return urd::Members<&DenotationBitset::num_objects, &DenotationBitset::words> {}; }
};

/// A concept's or role's values as its record holds them, flat: the words of its bits, which its built form
/// (DenotationBitset) writes.
template<ConceptOrRoleTag Category>
struct DenotationWords
{
    urd::flat_vector<ygg::uint_t> words;

    /// The words of built bits (a builder's, written into its record: urd's AssignsBuilt).
    DenotationWords& operator=(const DenotationBitset<Category>& bits)
    {
        words.set(bits.words.begin(), bits.words.end());
        return *this;
    }

    void clear() noexcept { words.clear(); }

    static constexpr auto identifying_members() noexcept { return urd::Members<&DenotationWords::words> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&DenotationWords::words> {}; }
};

/// The pool of a denotation repository's denotations of Category: each record packed as one row (its num_objects, then
/// its words), stored once (hashed and deduplicated by the pool) under its handle, whose number is its key.
template<ConceptOrRoleTag Category>
class DenotationPool;

/// What a lookup of a stored denotation hands out (urd::stored_ref_of): its pool and its key there, by value; its
/// fields read in place from its row (urd::stored_field).
template<ConceptOrRoleTag Category, typename Pool>
class StoredDenotation
{
    const Pool* m_pool;
    urd::key<Denotation<Category>> m_key;

public:
    StoredDenotation(const Pool& pool, urd::key<Denotation<Category>> key) noexcept : m_pool(&pool), m_key(key) {}

    [[nodiscard]] const Pool& pool() const noexcept { return *m_pool; }

    [[nodiscard]] urd::key<Denotation<Category>> key() const noexcept { return m_key; }
};

template<ConceptOrRoleTag Category>
class DenotationPool
{
public:
    using Row = urd::Store<urd::sequence<ygg::uint_t>, urd::frozen>;
    using Rows = urd::Store<urd::pool<Row>, urd::single_threaded>;
    using Handle = typename Rows::handle_type;
    using Key = urd::key<Denotation<Category>>;

    /// The store of a denotation repository's denotations of Category (urd's InsertsRecordsOf, FindsRecordsOf,
    /// HandsOutRecordsOf, ContainsKeysOf): a record packed into one row, stored once under its handle.
    template<typename Make>
    std::pair<Key, bool> insert(const urd::data<Denotation<Category>>& record, Make&& /*make*/)
    {
        pack(record, m_buffer);
        const auto [handle, inserted] = m_rows.insert(m_buffer);
        return { Key(handle.value), inserted };
    }

    [[nodiscard]] Key find(const urd::data<Denotation<Category>>& record) const
    {
        auto row = urd::Store<urd::sequence<ygg::uint_t>>();
        pack(record, row);
        return Key(m_rows.find(row).value);
    }

    [[nodiscard]] StoredDenotation<Category, DenotationPool> operator[](Key key) const noexcept { return { *this, key }; }

    [[nodiscard]] bool contains(Key key) const noexcept { return !key.is_none() && key.value < m_rows.size(); }

    /// The values of a stored denotation, read in place from its row.
    [[nodiscard]] DenotationBits<Category> bits(Key key) const
    {
        const auto row = m_rows.at(Handle(key.value));
        const auto words = std::span<const ygg::uint_t>(row.data(), row.size());  // the number of objects, then the words
        return DenotationBits<Category>(words.subspan(1).data(), words.front());
    }

    /// The denotations in its pool, none below it (not layered: urd::count_at's size and base).
    [[nodiscard]] std::size_t size() const noexcept { return m_rows.size(); }

    [[nodiscard]] std::size_t base() const noexcept { return 0; }

    void clear() { m_rows.clear(); }

    /// Its memory (urd::memory_usage): its rows and its buffer.
    static constexpr auto memory_members() noexcept { return urd::Members<&DenotationPool::m_rows, &DenotationPool::m_buffer> {}; }

private:
    /// A record as its row: its num_objects, then its words.
    static void pack(const urd::data<Denotation<Category>>& record, urd::Store<urd::sequence<ygg::uint_t>>& row)
    {
        row.clear();
        row.push_back(record.num_objects);
        for (const auto word : record.values.words)
            row.push_back(word);
    }

    Rows m_rows;
    urd::Store<urd::sequence<ygg::uint_t>> m_buffer;
};

}  // namespace runir::kr::dl::semantics

template<runir::kr::dl::ConceptOrRoleTag Category>
inline constexpr bool std::ranges::enable_borrowed_range<runir::kr::dl::semantics::DenotationBits<Category>> = true;

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
inline constexpr bool std::ranges::enable_borrowed_range<runir::kr::dl::semantics::ResolvedDenotationBits<Category, C>> = true;

namespace urd
{

/// A lookup of a stored concept or role denotation hands out its reader (StoredDenotation).
template<runir::kr::dl::ConceptOrRoleTag Category>
struct stored_ref_of<runir::kr::dl::semantics::Denotation<Category>>
{
    using type = runir::kr::dl::semantics::StoredDenotation<Category, runir::kr::dl::semantics::DenotationPool<Category>>;
};

/// A stored denotation's fields are read from its pool's row: its num_objects, and its values in place.
template<auto Member, runir::kr::dl::ConceptOrRoleTag Category, typename Pool>
struct stored_field<Member, runir::kr::dl::semantics::StoredDenotation<Category, Pool>>
{
    [[nodiscard]] static auto read(const runir::kr::dl::semantics::StoredDenotation<Category, Pool>& stored)
    {
        const auto bits = stored.pool().bits(stored.key());
        if constexpr (std::same_as<member_type_t<Member>, ygg::uint_t>)
            return bits.num_objects();
        else if constexpr (std::same_as<member_type_t<Member>, runir::kr::dl::semantics::DenotationWords<Category>>)
            return bits;
        else
            static_assert(false, "a stored denotation's fields are its num_objects and its values");
    }
};

/// A concept's or role's values are built as their bitset (its builder's num_objects and words, set in place).
template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
struct built_form<runir::kr::dl::semantics::DenotationWords<Category>, C>
{
    using type = runir::kr::dl::semantics::DenotationBitset<Category>;
};

/// A denotation's values, read in place, resolved through a view's context as the views of what they denote.
template<runir::kr::dl::ConceptOrRoleTag Category>
struct resolution<runir::kr::dl::semantics::DenotationBits<Category>>
{
    template<typename C>
    static runir::kr::dl::semantics::ResolvedDenotationBits<Category, C> of(runir::kr::dl::semantics::DenotationBits<Category> bits, const C& context) noexcept
    { return runir::kr::dl::semantics::ResolvedDenotationBits<Category, C>(bits, &context); }
};

/// A builder's values, likewise.
template<runir::kr::dl::ConceptOrRoleTag Category>
struct resolution<runir::kr::dl::semantics::DenotationBitset<Category>>
{
    template<typename C>
    static runir::kr::dl::semantics::ResolvedDenotationBits<Category, C> of(const runir::kr::dl::semantics::DenotationBitset<Category>& bitset,
                                                                            const C& context) noexcept
    { return runir::kr::dl::semantics::ResolvedDenotationBits<Category, C>(bitset.bits(), &context); }

    /// Written from a stored denotation's values (its words, read in place): what a builder is built from (urd::copy).
    static runir::kr::dl::semantics::DenotationBitset<Category> stored(runir::kr::dl::semantics::DenotationBits<Category> bits)
    {
        auto result = runir::kr::dl::semantics::DenotationBitset<Category>();
        result.assign(bits);
        return result;
    }
};

}  // namespace urd

#endif
