#ifndef RUNIR_GRAMMAR_NON_TERMINAL_DATA_HPP_
#define RUNIR_GRAMMAR_NON_TERMINAL_DATA_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::dl::grammar::NonTerminal<Category>>
{
    urd::index<runir::kr::dl::grammar::NonTerminal<Category>> index;
    urd::flat_string name;

    data() = default;

    explicit data(urd::flat_string name_) : index(), name(std::move(name_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"name", &data::name>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name> {}; }
};

}  // namespace urd

#endif
