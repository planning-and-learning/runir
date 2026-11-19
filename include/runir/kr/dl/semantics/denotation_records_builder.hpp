#ifndef RUNIR_KR_DL_SEMANTICS_DENOTATION_RECORDS_BUILDER_HPP_
#define RUNIR_KR_DL_SEMANTICS_DENOTATION_RECORDS_BUILDER_HPP_

#include "runir/kr/dl/semantics/denotation_data.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of DenotationBlocks: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::semantics::DenotationBlocks, C>
{
    urd::index<runir::kr::dl::semantics::DenotationBlocks> index;
    urd::built_t<urd::flat_vector<ygg::uint_t>, C> words;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"words", &builder::words>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
