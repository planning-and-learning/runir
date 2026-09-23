#ifndef RUNIR_KR_PS_EXT_DETAIL_CHOICE_PROOFS_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_CHOICE_PROOFS_HPP_

#include <vector>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::ps::ext::detail
{

enum class ChoiceId : ygg::uint_t
{
};

/// One success flag per Choose rule at a program state, retained after its binding cursor is gone.
class ChoiceProofs
{
private:
    std::vector<bool> m_satisfied;

public:
    ChoiceId create()
    {
        const auto id = static_cast<ChoiceId>(ygg::to_uint_t(m_satisfied.size()));
        m_satisfied.push_back(false);
        return id;
    }

    bool is_satisfied(ChoiceId id) const { return m_satisfied[static_cast<ygg::uint_t>(id)]; }

    /// Return true only on the first success, so the parent requirement is credited once.
    bool satisfy(ChoiceId id)
    {
        const auto index = static_cast<ygg::uint_t>(id);
        if (m_satisfied[index])
            return false;
        m_satisfied[index] = true;
        return true;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
