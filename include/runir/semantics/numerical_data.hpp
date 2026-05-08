#ifndef RUNIR_SEMANTICS_NUMERICAL_DATA_HPP_
#define RUNIR_SEMANTICS_NUMERICAL_DATA_HPP_

#include "runir/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<>
struct Data<runir::Numerical<runir::CountTag>>
{
    using ConstructorVariant = ::cista::offset::variant<Index<runir::Constructor<runir::ConceptTag>>, Index<runir::Constructor<runir::RoleTag>>>;

    Index<runir::Numerical<runir::CountTag>> index;
    ConstructorVariant arg;

    Data() = default;
    explicit Data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

template<>
struct Data<runir::Numerical<runir::DistanceTag>> :
    runir::semantics::TernaryData<runir::Numerical<runir::DistanceTag>,
                                  runir::Constructor<runir::ConceptTag>,
                                  runir::Constructor<runir::RoleTag>,
                                  runir::Constructor<runir::ConceptTag>>
{
    using Base = runir::semantics::TernaryData<runir::Numerical<runir::DistanceTag>,
                                               runir::Constructor<runir::ConceptTag>,
                                               runir::Constructor<runir::RoleTag>,
                                               runir::Constructor<runir::ConceptTag>>;
    using Base::Base;
};

static_assert(!uses_trivial_storage_v<runir::Numerical<runir::CountTag>>);

}

#endif
