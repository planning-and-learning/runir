#ifndef RUNIR_SEMANTICS_DENOTATION_DATA_HPP_
#define RUNIR_SEMANTICS_DENOTATION_DATA_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <tuple>
#include <utility>
#include <yggdrasil/core/config.hpp>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> index;
    bool value = false;

    data() = default;
    explicit data(bool value_) noexcept : value(value_) {}

    auto get_data() noexcept -> bool& { return value; }
    [[nodiscard]] auto get_data() const noexcept -> const bool& { return value; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::value> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::value> {}; }
};

template<>
struct data<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> index;
    ygg::uint_t value = 0;

    data() = default;
    explicit data(ygg::uint_t value_) noexcept : value(value_) {}

    auto get_data() noexcept -> ygg::uint_t& { return value; }
    [[nodiscard]] auto get_data() const noexcept -> const ygg::uint_t& { return value; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::value> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::value> {}; }
};

template<>
struct data<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> index;
    ygg::uint_t num_objects = 0;
    ygg::uint_t vec_index = 0;

    data() = default;
    data(ygg::uint_t num_objects_, ygg::uint_t vec_index_) noexcept : num_objects(num_objects_), vec_index(vec_index_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::num_objects, &data::vec_index> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::num_objects, &data::vec_index> {}; }
};

template<>
struct data<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> index;
    ygg::uint_t num_objects = 0;
    ygg::uint_t vec_index = 0;

    data() = default;
    data(ygg::uint_t num_objects_, ygg::uint_t vec_index_) noexcept : num_objects(num_objects_), vec_index(vec_index_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::num_objects, &data::vec_index> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::num_objects, &data::vec_index> {}; }
};


}

#endif
