#ifndef RUNIR_GRAPHS_BGL_PROPERTY_MAPS_HPP_
#define RUNIR_GRAPHS_BGL_PROPERTY_MAPS_HPP_

#include "runir/graphs/declarations.hpp"

#include <boost/property_map/property_map.hpp>
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

template<std::unsigned_integral Key, typename Value>
auto get(const IdentityReadPropertyMap<Key, Value>&, Key key) noexcept -> Value
{
    return static_cast<Value>(key);
}

template<std::unsigned_integral Key, typename Value>
class DenseReadPropertyMap
{
private:
    std::reference_wrapper<const std::vector<Value>> m_data;

public:
    using value_type = Value;
    using key_type = Key;
    using reference = const Value&;
    using category = boost::readable_property_map_tag;

    explicit DenseReadPropertyMap(const std::vector<Value>& data) noexcept : m_data(data) {}

    auto get(Key key) const -> const Value& { return m_data.get().at(key); }
    auto get_data() const noexcept -> const std::vector<Value>& { return m_data.get(); }
};

template<std::unsigned_integral Key, typename Value>
auto get(const DenseReadPropertyMap<Key, Value>& map, Key key) -> const Value&
{
    return map.get(key);
}

template<std::unsigned_integral Key, typename Value>
class DenseReadWritePropertyMap
{
private:
    std::reference_wrapper<std::vector<Value>> m_data;

public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value&;
    using category = boost::read_write_property_map_tag;

    explicit DenseReadWritePropertyMap(std::vector<Value>& data) noexcept : m_data(data) {}

    auto get(Key key) const -> const Value& { return m_data.get().at(key); }
    void put(Key key, const Value& value) { m_data.get().at(key) = value; }
    auto get_data() noexcept -> std::vector<Value>& { return m_data.get(); }
    auto get_data() const noexcept -> const std::vector<Value>& { return m_data.get(); }
};

template<std::unsigned_integral Key, typename Value>
auto get(const DenseReadWritePropertyMap<Key, Value>& map, Key key) -> const Value&
{
    return map.get(key);
}

template<std::unsigned_integral Key, typename Value>
void put(DenseReadWritePropertyMap<Key, Value>& map, Key key, const Value& value)
{
    map.put(key, value);
}

template<std::unsigned_integral Key, typename Value>
class SparseReadPropertyMap
{
private:
    std::reference_wrapper<const tyr::UnorderedMap<Key, Value>> m_data;

public:
    using value_type = Value;
    using key_type = Key;
    using reference = const Value&;
    using category = boost::readable_property_map_tag;

    explicit SparseReadPropertyMap(const tyr::UnorderedMap<Key, Value>& data) noexcept : m_data(data) {}

    auto get(Key key) const -> const Value& { return m_data.get().at(key); }
    auto get_data() const noexcept -> const tyr::UnorderedMap<Key, Value>& { return m_data.get(); }
};

template<std::unsigned_integral Key, typename Value>
auto get(const SparseReadPropertyMap<Key, Value>& map, Key key) -> const Value&
{
    return map.get(key);
}

template<std::unsigned_integral Key, typename Value>
class SparseReadWritePropertyMap
{
private:
    std::reference_wrapper<tyr::UnorderedMap<Key, Value>> m_data;

public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value&;
    using category = boost::read_write_property_map_tag;

    explicit SparseReadWritePropertyMap(tyr::UnorderedMap<Key, Value>& data) noexcept : m_data(data) {}

    auto get(Key key) const -> const Value& { return m_data.get().at(key); }
    void put(Key key, const Value& value) { m_data.get()[key] = value; }
    auto get_data() noexcept -> tyr::UnorderedMap<Key, Value>& { return m_data.get(); }
    auto get_data() const noexcept -> const tyr::UnorderedMap<Key, Value>& { return m_data.get(); }
};

template<std::unsigned_integral Key, typename Value>
auto get(const SparseReadWritePropertyMap<Key, Value>& map, Key key) -> const Value&
{
    return map.get(key);
}

template<std::unsigned_integral Key, typename Value>
void put(SparseReadWritePropertyMap<Key, Value>& map, Key key, const Value& value)
{
    map.put(key, value);
}

template<std::unsigned_integral Key, typename Value>
class DenseBasicMatrix
{
private:
    std::reference_wrapper<std::vector<std::vector<Value>>> m_data;

public:
    explicit DenseBasicMatrix(std::vector<std::vector<Value>>& data) noexcept : m_data(data) {}

    auto operator[](Key key) -> std::vector<Value>& { return m_data.get().at(key); }
    auto operator[](Key key) const -> const std::vector<Value>& { return m_data.get().at(key); }
    auto get_data() const noexcept -> const std::vector<std::vector<Value>>& { return m_data.get(); }
};

template<std::unsigned_integral Key, typename Value>
class SparseBasicMatrix
{
private:
    std::reference_wrapper<tyr::UnorderedMap<Key, tyr::UnorderedMap<Key, Value>>> m_data;

public:
    explicit SparseBasicMatrix(tyr::UnorderedMap<Key, tyr::UnorderedMap<Key, Value>>& data) noexcept : m_data(data) {}

    auto operator[](Key key) -> tyr::UnorderedMap<Key, Value>& { return m_data.get()[key]; }
    auto operator[](Key key) const -> const tyr::UnorderedMap<Key, Value>& { return m_data.get().at(key); }
    auto get_data() const noexcept -> const tyr::UnorderedMap<Key, tyr::UnorderedMap<Key, Value>>& { return m_data.get(); }
};

}  // namespace runir::graphs::bgl

#endif
