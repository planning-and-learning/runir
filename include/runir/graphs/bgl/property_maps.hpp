#ifndef RUNIR_GRAPHS_BGL_PROPERTY_MAPS_HPP_
#define RUNIR_GRAPHS_BGL_PROPERTY_MAPS_HPP_

#include "runir/graphs/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include <boost/property_map/property_map.hpp>
#include <concepts>
#include <functional>
#include <vector>

namespace runir::graphs::bgl
{

template<std::unsigned_integral Key, typename Value = Key>
struct IdentityReadPropertyMap
{
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::readable_property_map_tag;
};

template<std::unsigned_integral Key, std::copyable Value>
auto get(const IdentityReadPropertyMap<Key, Value>&, Key key) noexcept -> Value
{ return static_cast<Value>(key); }

template<std::unsigned_integral Key, typename Value>
class DenseReadPropertyMap
{
private:
    std::reference_wrapper<const urd::Store<urd::sequence<Value>>> m_data;

public:
    using value_type = Value;
    using key_type = Key;
    using reference = const Value&;
    using category = boost::readable_property_map_tag;

    explicit DenseReadPropertyMap(const urd::Store<urd::sequence<Value>>& data) noexcept : m_data(data) {}

    [[nodiscard]] auto get(Key key) const -> const Value& { return m_data.get().at(key); }
    [[nodiscard]] auto get_data() const noexcept -> const urd::Store<urd::sequence<Value>>& { return m_data.get(); }
};

template<std::unsigned_integral Key, std::copyable Value>
auto get(const DenseReadPropertyMap<Key, Value>& map, Key key) -> const Value&
{ return map.get(key); }

template<std::unsigned_integral Key, typename Value>
class DenseReadWritePropertyMap
{
private:
    std::reference_wrapper<urd::Store<urd::sequence<Value>>> m_data;

public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value&;
    using category = boost::read_write_property_map_tag;

    explicit DenseReadWritePropertyMap(urd::Store<urd::sequence<Value>>& data) noexcept : m_data(data) {}

    [[nodiscard]] auto get(Key key) const -> const Value& { return m_data.get().at(key); }
    void put(Key key, const Value& value) { m_data.get().at(key) = value; }
    auto get_data() noexcept -> urd::Store<urd::sequence<Value>>& { return m_data.get(); }
    [[nodiscard]] auto get_data() const noexcept -> const urd::Store<urd::sequence<Value>>& { return m_data.get(); }
};

template<std::unsigned_integral Key, std::copyable Value>
auto get(const DenseReadWritePropertyMap<Key, Value>& map, Key key) -> const Value&
{ return map.get(key); }

template<std::unsigned_integral Key, std::copyable Value>
void put(DenseReadWritePropertyMap<Key, Value>& map, Key key, const Value& value)
{ map.put(key, value); }

template<std::unsigned_integral Key, typename Value>
class SparseReadPropertyMap
{
private:
    std::reference_wrapper<const urd::Store<urd::map<Key, Value>>> m_data;

public:
    using value_type = Value;
    using key_type = Key;
    using reference = const Value&;
    using category = boost::readable_property_map_tag;

    explicit SparseReadPropertyMap(const urd::Store<urd::map<Key, Value>>& data) noexcept : m_data(data) {}

    [[nodiscard]] auto get(Key key) const -> const Value& { return m_data.get().at(key); }
    [[nodiscard]] auto get_data() const noexcept -> const urd::Store<urd::map<Key, Value>>& { return m_data.get(); }
};

template<std::unsigned_integral Key, std::copyable Value>
auto get(const SparseReadPropertyMap<Key, Value>& map, Key key) -> const Value&
{ return map.get(key); }

template<std::unsigned_integral Key, typename Value>
class SparseReadWritePropertyMap
{
private:
    std::reference_wrapper<urd::Store<urd::map<Key, Value>>> m_data;

public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value&;
    using category = boost::read_write_property_map_tag;

    explicit SparseReadWritePropertyMap(urd::Store<urd::map<Key, Value>>& data) noexcept : m_data(data) {}

    [[nodiscard]] auto get(Key key) const -> const Value& { return m_data.get().at(key); }
    void put(Key key, const Value& value) { m_data.get()[key] = value; }
    auto get_data() noexcept -> urd::Store<urd::map<Key, Value>>& { return m_data.get(); }
    [[nodiscard]] auto get_data() const noexcept -> const urd::Store<urd::map<Key, Value>>& { return m_data.get(); }
};

template<std::unsigned_integral Key, std::copyable Value>
auto get(const SparseReadWritePropertyMap<Key, Value>& map, Key key) -> const Value&
{ return map.get(key); }

template<std::unsigned_integral Key, std::copyable Value>
void put(SparseReadWritePropertyMap<Key, Value>& map, Key key, const Value& value)
{ map.put(key, value); }

template<std::unsigned_integral Key, typename Value>
class DenseBasicMatrix
{
private:
    std::reference_wrapper<urd::Store<urd::sequence<urd::Store<urd::sequence<Value>>>>> m_data;

public:
    explicit DenseBasicMatrix(urd::Store<urd::sequence<urd::Store<urd::sequence<Value>>>>& data) noexcept : m_data(data) {}

    auto operator[](Key key) -> urd::Store<urd::sequence<Value>>& { return m_data.get().at(key); }
    auto operator[](Key key) const -> const urd::Store<urd::sequence<Value>>& { return m_data.get().at(key); }
    [[nodiscard]] auto get_data() const noexcept -> const urd::Store<urd::sequence<urd::Store<urd::sequence<Value>>>>& { return m_data.get(); }
};

template<std::unsigned_integral Key, typename Value>
class SparseBasicMatrix
{
private:
    std::reference_wrapper<urd::Store<urd::map<Key, urd::Store<urd::map<Key, Value>>>>> m_data;

public:
    explicit SparseBasicMatrix(urd::Store<urd::map<Key, urd::Store<urd::map<Key, Value>>>>& data) noexcept : m_data(data) {}

    auto operator[](Key key) -> urd::Store<urd::map<Key, Value>>& { return m_data.get()[key]; }
    auto operator[](Key key) const -> const urd::Store<urd::map<Key, Value>>& { return m_data.get().at(key); }
    [[nodiscard]] auto get_data() const noexcept -> const urd::Store<urd::map<Key, urd::Store<urd::map<Key, Value>>>>& { return m_data.get(); }
};

}  // namespace runir::graphs::bgl

#endif
