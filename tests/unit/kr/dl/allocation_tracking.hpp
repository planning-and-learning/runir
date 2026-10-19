#ifndef RUNIR_TESTS_KR_DL_ALLOCATION_TRACKING_HPP_
#define RUNIR_TESTS_KR_DL_ALLOCATION_TRACKING_HPP_

#include <cstddef>

/// Counts the allocations of the current thread while a Scope is alive. The replacement global operators new and
/// delete that feed it are defined once, in query_allocations.cpp.
namespace allocation_tracking
{
struct Counts
{
    size_t allocated = 0;
    size_t deallocated = 0;
};

extern thread_local bool enabled;
extern thread_local Counts counts;

void* allocate(size_t bytes);
void deallocate(void* pointer) noexcept;
void* allocate_aligned(size_t bytes, size_t alignment);
void deallocate_aligned(void* pointer) noexcept;

class Scope
{
public:
    Scope()
    {
        counts = {};
        enabled = true;
    }
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    ~Scope() { enabled = false; }

    Counts finish() noexcept
    {
        enabled = false;
        return counts;
    }
};
}  // namespace allocation_tracking

#endif
