#ifndef RUNIR_KR_PS_EXT_DETAIL_POOLED_SHARED_OWNER_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_POOLED_SHARED_OWNER_HPP_

#include <memory>
#include <utility>
#include <yggdrasil/containers/shared_object_pool.hpp>

namespace runir::kr::ps::ext::detail
{

/// Shared pool slots retain their buffers, but must release child owners before reuse.
/// The pool has one shared_ptr control block; each checked-out slot uses Ygg's inline refcount.
template<typename T>
class PooledSharedOwner
{
    std::shared_ptr<ygg::SharedObjectPool<T>> m_pool;
    ygg::SharedObjectPoolPtr<T> m_pointer;

public:
    PooledSharedOwner() noexcept = default;
    explicit PooledSharedOwner(std::shared_ptr<ygg::SharedObjectPool<T>> pool) : m_pool(std::move(pool)), m_pointer(m_pool->get_or_allocate()) {}
    PooledSharedOwner(const PooledSharedOwner&) noexcept = default;
    PooledSharedOwner(PooledSharedOwner&&) noexcept = default;
    PooledSharedOwner& operator=(PooledSharedOwner other) noexcept
    {
        swap(other);
        return *this;
    }
    ~PooledSharedOwner() { reset(); }

    void swap(PooledSharedOwner& other) noexcept
    {
        m_pool.swap(other.m_pool);
        std::swap(m_pointer, other.m_pointer);
    }
    void reset() noexcept
    {
        if (m_pointer && m_pointer.ref_count() == 1)
            m_pointer->release_owners();
        m_pointer = {};
        m_pool.reset();
    }

    T* get() const noexcept { return m_pointer.get(); }
    T& operator*() const noexcept { return *m_pointer; }
    T* operator->() const noexcept { return m_pointer.get(); }
    explicit operator bool() const noexcept { return static_cast<bool>(m_pointer); }
    size_t ref_count() const noexcept { return m_pointer ? m_pointer.ref_count() : 0; }
};

}  // namespace runir::kr::ps::ext::detail

#endif
