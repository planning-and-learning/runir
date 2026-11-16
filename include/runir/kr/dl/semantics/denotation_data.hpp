#ifndef RUNIR_SEMANTICS_DENOTATION_DATA_HPP_
#define RUNIR_SEMANTICS_DENOTATION_DATA_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <tuple>
#include <type_traits>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::dl::semantics
{
/// The blocks of a concept's or role's denotation, interned in its DenotationRepository: the record's blocks field is their handle.
using DenotationBlocks = urd::Store<urd::sequence<ygg::uint_t>, urd::frozen>;
}

namespace urd
{

template<>
struct data<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> index;
    bool value = false;

    data() = default;
    explicit data(bool value_) noexcept : value(value_) {}
    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &data::value>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::value> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::value> {}; }
};

template<>
struct data<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> index;
    ygg::uint_t value = 0;

    data() = default;
    explicit data(ygg::uint_t value_) noexcept : value(value_) {}
    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &data::value>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::value> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::value> {}; }
};

template<>
struct data<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> index;
    ygg::uint_t num_objects = 0;
    urd::index<runir::kr::dl::semantics::DenotationBlocks> blocks;

    data() = default;
    data(ygg::uint_t num_objects_, urd::index<runir::kr::dl::semantics::DenotationBlocks> blocks_) noexcept : num_objects(num_objects_), blocks(blocks_) {}
    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"num_objects", &data::num_objects>, urd::field<"blocks", &data::blocks>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::num_objects, &data::blocks> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::num_objects, &data::blocks> {}; }
};

template<>
struct data<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> index;
    ygg::uint_t num_objects = 0;
    urd::index<runir::kr::dl::semantics::DenotationBlocks> blocks;

    data() = default;
    data(ygg::uint_t num_objects_, urd::index<runir::kr::dl::semantics::DenotationBlocks> blocks_) noexcept : num_objects(num_objects_), blocks(blocks_) {}
    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"num_objects", &data::num_objects>, urd::field<"blocks", &data::blocks>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::num_objects, &data::blocks> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::num_objects, &data::blocks> {}; }
};

}

#endif
