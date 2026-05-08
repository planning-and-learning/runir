#ifndef RUNIR_SEMANTICS_DENOTATION_DATA_HPP_
#define RUNIR_SEMANTICS_DENOTATION_DATA_HPP_

#include "runir/config.hpp"
#include "runir/semantics/denotation_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::semantics::Denotation<runir::BooleanTag>>
{
    Index<runir::semantics::Denotation<runir::BooleanTag>> index;
    bool value = false;

    Data() = default;
    explicit Data(bool value_) noexcept : value(value_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }

    auto get_data() noexcept -> bool& { return value; }
    auto get_data() const noexcept -> const bool& { return value; }
};

template<>
struct Data<runir::semantics::Denotation<runir::NumericalTag>>
{
    Index<runir::semantics::Denotation<runir::NumericalTag>> index;
    runir::uint_t value = 0;

    Data() = default;
    explicit Data(runir::uint_t value_) noexcept : value(value_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }

    auto get_data() noexcept -> runir::uint_t& { return value; }
    auto get_data() const noexcept -> const runir::uint_t& { return value; }
};

template<>
struct Data<runir::semantics::Denotation<runir::ConceptTag>>
{
    Index<runir::semantics::Denotation<runir::ConceptTag>> index;
    runir::uint_t num_objects = 0;
    runir::uint_t vec_index = 0;

    Data() = default;
    Data(runir::uint_t num_objects_, runir::uint_t vec_index_) noexcept : num_objects(num_objects_), vec_index(vec_index_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(num_objects);
        tyr::clear(vec_index);
    }

    auto cista_members() const noexcept { return std::tie(index, num_objects, vec_index); }
    auto identifying_members() const noexcept { return std::tie(num_objects, vec_index); }
};

template<>
struct Data<runir::semantics::Denotation<runir::RoleTag>>
{
    Index<runir::semantics::Denotation<runir::RoleTag>> index;
    runir::uint_t num_objects = 0;
    runir::uint_t vec_index = 0;

    Data() = default;
    Data(runir::uint_t num_objects_, runir::uint_t vec_index_) noexcept : num_objects(num_objects_), vec_index(vec_index_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(num_objects);
        tyr::clear(vec_index);
    }

    auto cista_members() const noexcept { return std::tie(index, num_objects, vec_index); }
    auto identifying_members() const noexcept { return std::tie(num_objects, vec_index); }
};

static_assert(uses_trivial_storage_v<runir::semantics::Denotation<runir::BooleanTag>>);
static_assert(uses_trivial_storage_v<runir::semantics::Denotation<runir::NumericalTag>>);
static_assert(uses_trivial_storage_v<runir::semantics::Denotation<runir::ConceptTag>>);
static_assert(uses_trivial_storage_v<runir::semantics::Denotation<runir::RoleTag>>);

}

#endif
