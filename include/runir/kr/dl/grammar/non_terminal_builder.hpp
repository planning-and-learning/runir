#ifndef RUNIR_KR_DL_GRAMMAR_NON_TERMINAL_BUILDER_HPP_
#define RUNIR_KR_DL_GRAMMAR_NON_TERMINAL_BUILDER_HPP_

#include "runir/kr/dl/grammar/non_terminal_data.hpp"
#include "runir/kr/dl/grammar/non_terminal_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of NonTerminal: its record's members, each in its built form (urd::built_t).
template<runir::kr::dl::CategoryTag Category, typename C>
struct builder<runir::kr::dl::grammar::NonTerminal<Category>, C>
{
    urd::index<runir::kr::dl::grammar::NonTerminal<Category>> index;
    urd::built_t<urd::flat_string, C> name;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"name", &builder::name>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::name> {}; }
};

}  // namespace urd

#endif
