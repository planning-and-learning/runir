#ifndef RUNIR_SEMANTICS_DENOTATION_DATA_HPP_
#define RUNIR_SEMANTICS_DENOTATION_DATA_HPP_

#include "runir/kr/dl/semantics/denotation_index.hpp"

#include <tuple>
#include <utility>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>
{
    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> index;
    bool value = false;

    Data() = default;
    explicit Data(bool value_) noexcept : value(value_) {}

    auto get_data() noexcept -> bool& { return value; }
    auto get_data() const noexcept -> const bool& { return value; }

    auto cista_members() noexcept { return std::tie(index, value); }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<>
struct Data<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>
{
    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> index;
    ygg::uint_t value = 0;

    Data() = default;
    explicit Data(ygg::uint_t value_) noexcept : value(value_) {}

    auto get_data() noexcept -> ygg::uint_t& { return value; }
    auto get_data() const noexcept -> const ygg::uint_t& { return value; }

    auto cista_members() noexcept { return std::tie(index, value); }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<>
struct Data<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>
{
    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> index;
    ygg::uint_t num_objects = 0;
    ygg::uint_t vec_index = 0;

    Data() = default;
    Data(ygg::uint_t num_objects_, ygg::uint_t vec_index_) noexcept : num_objects(num_objects_), vec_index(vec_index_) {}

    auto cista_members() noexcept { return std::tie(index, num_objects, vec_index); }
    auto cista_members() const noexcept { return std::tie(index, num_objects, vec_index); }
    auto identifying_members() const noexcept { return std::tie(num_objects, vec_index); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<>
struct Data<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>
{
    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> index;
    ygg::uint_t num_objects = 0;
    ygg::uint_t vec_index = 0;

    Data() = default;
    Data(ygg::uint_t num_objects_, ygg::uint_t vec_index_) noexcept : num_objects(num_objects_), vec_index(vec_index_) {}

    auto cista_members() noexcept { return std::tie(index, num_objects, vec_index); }
    auto cista_members() const noexcept { return std::tie(index, num_objects, vec_index); }
    auto identifying_members() const noexcept { return std::tie(num_objects, vec_index); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

static_assert(uses_trivial_storage_v<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>);
static_assert(uses_trivial_storage_v<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>);
static_assert(uses_trivial_storage_v<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>);
static_assert(uses_trivial_storage_v<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>);

}

#endif
