#ifndef RUNIR_SEMANTICS_DENOTATION_DATA_HPP_
#define RUNIR_SEMANTICS_DENOTATION_DATA_HPP_

#include "runir/common/config.hpp"
#include "runir/knowledge_representation/dl/semantics/denotation_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>
{
    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> index;
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
struct Data<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>
{
    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> index;
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
struct Data<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>
{
    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> index;
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
struct Data<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>
{
    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> index;
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

static_assert(uses_trivial_storage_v<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>);
static_assert(uses_trivial_storage_v<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>);
static_assert(uses_trivial_storage_v<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>);
static_assert(uses_trivial_storage_v<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>);

}

#endif
