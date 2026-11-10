#ifndef RUNIR_KR_DL_DETAIL_GRAMMAR_VIEWS_HPP_
#define RUNIR_KR_DL_DETAIL_GRAMMAR_VIEWS_HPP_

#include <tuple>
#include <urd/elements/flat.hpp>
#include <urd/elements/view.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace runir::kr::dl::detail
{

/// Whether T is a grammar's nonterminal (dl::grammar, dl::cnf_grammar): each grammar says so for its own.
template<typename T>
inline constexpr bool is_grammar_non_terminal = false;

/// Whether T is a grammar's derivation rule, likewise.
template<typename T>
inline constexpr bool is_grammar_derivation_rule = false;

}  // namespace runir::kr::dl::detail

namespace urd
{
template<typename T, typename V>
    requires runir::kr::dl::detail::is_grammar_non_terminal<T>
struct view_mixin<T, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_name() const noexcept { return self().data().name; }
};

template<typename T, typename V>
    requires runir::kr::dl::detail::is_grammar_derivation_rule<T>
struct view_mixin<T, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_lhs() const noexcept { return self().template member<&urd::record_of<V>::lhs>(); }
    auto get_rhs() const noexcept { return self().template member<&urd::record_of<V>::rhs>(); }
};
}  // namespace urd

#endif
