#ifndef RUNIR_SEMANTICS_NUMERICAL_DATA_HPP_
#define RUNIR_SEMANTICS_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::dl::Numerical<runir::kr::dl::CountTag>>
{
    using ConstructorVariant =
        ::cista::offset::variant<Index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>, Index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>>;

    Index<runir::kr::dl::Numerical<runir::kr::dl::CountTag>> index;
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
struct Data<runir::kr::dl::Numerical<runir::kr::dl::DistanceTag>> :
    runir::kr::dl::semantics::TernaryData<runir::kr::dl::Numerical<runir::kr::dl::DistanceTag>,
                                          runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>,
                                          runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                          runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::TernaryData<runir::kr::dl::Numerical<runir::kr::dl::DistanceTag>,
                                                       runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>,
                                                       runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                                       runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

static_assert(!uses_trivial_storage_v<runir::kr::dl::Numerical<runir::kr::dl::CountTag>>);

}

#endif
