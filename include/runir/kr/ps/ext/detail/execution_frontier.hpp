#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_FRONTIER_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_FRONTIER_HPP_

#include "runir/kr/ps/ext/detail/execution_step.hpp"

#include <cstddef>
#include <limits>
#include <utility>
#include <variant>
#include <vector>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
struct PendingChoice
{
    ygg::Index<ProgramState<Kind>> state;
    std::variant<Choice<runir::kr::dl::ConceptTag>, Choice<runir::kr::dl::RoleTag>> choice;
};

/// Pending work stored as an append-only chain until rollback.
template<tyr::TaskKind Kind>
class ExecutionFrontier
{
public:
    using Work = std::variant<ygg::Index<ProgramState<Kind>>, PendingChoice<Kind>>;

    struct Checkpoint
    {
        std::size_t head;
        std::size_t storage_size;
    };

private:
    static constexpr auto no_entry = std::numeric_limits<std::size_t>::max();

    struct Entry
    {
        Work work;
        std::size_t next;
    };

    std::vector<Entry> m_entries;
    std::size_t m_head = no_entry;

public:
    bool empty() const { return m_head == no_entry; }

    void push(Work work)
    {
        const auto next = m_head;
        m_head = m_entries.size();
        m_entries.push_back({ std::move(work), next });
    }

    Work pop()
    {
        const auto& entry = m_entries[m_head];
        m_head = entry.next;
        // Retain the entry and return a copy so rollback can replay the original choice cursor.
        return entry.work;
    }

    Checkpoint checkpoint() const { return { m_head, m_entries.size() }; }

    /// Restore the pending chain and discard entries appended after the snapshot.
    void restore(const Checkpoint& checkpoint)
    {
        m_head = checkpoint.head;
        m_entries.erase(m_entries.begin() + checkpoint.storage_size, m_entries.end());
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
