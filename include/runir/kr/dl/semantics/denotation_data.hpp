#ifndef RUNIR_SEMANTICS_DENOTATION_DATA_HPP_
#define RUNIR_SEMANTICS_DENOTATION_DATA_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_storage.hpp"

#include <tuple>
#include <type_traits>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>

namespace urd
{

template<>
struct data<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>
{
    urd::key<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> index;
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

    [[nodiscard]] urd::key<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> key() const noexcept
    { return urd::key<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>(index); }
};

template<>
struct data<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>
{
    urd::key<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> index;
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

    [[nodiscard]] urd::key<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> key() const noexcept
    { return urd::key<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>(index); }
};

/// A concept's or role's denotation: what it denotes over num_objects objects (values: a concept's objects, a role's
/// pairs of them), as the words of its bits. Its repository packs it into a pool of its own (DenotationPool), its
/// builder holds its bits (DenotationBitset), and a view reads them in place (DenotationBits).
template<runir::kr::dl::ConceptOrRoleTag Category>
struct data<runir::kr::dl::semantics::Denotation<Category>>
{
    urd::key<runir::kr::dl::semantics::Denotation<Category>> index;
    ygg::uint_t num_objects = 0;
    runir::kr::dl::semantics::DenotationWords<Category> values;

    data() = default;

    data(ygg::uint_t num_objects_, runir::kr::dl::semantics::DenotationWords<Category> values_) : index(), num_objects(num_objects_), values(std::move(values_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"num_objects", &data::num_objects>, urd::field<"values", &data::values>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::num_objects, &data::values> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::semantics::Denotation<Category>> key() const noexcept
    { return urd::key<runir::kr::dl::semantics::Denotation<Category>>(index); }
};

}

#endif
