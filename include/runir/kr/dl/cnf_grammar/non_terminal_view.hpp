#ifndef RUNIR_CNF_GRAMMAR_NON_TERMINAL_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_NON_TERMINAL_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/non_terminal_data.hpp"

#include <urd/elements/view.hpp>

namespace urd
{
template<runir::kr::dl::CategoryTag Category, typename V>
struct view_mixin<runir::kr::dl::cnf_grammar::NonTerminal<Category>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_name() const noexcept { return self().template member<&urd::record_of<V>::name>(); }
};
}  // namespace urd

#endif
