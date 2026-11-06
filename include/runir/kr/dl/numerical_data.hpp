#ifndef RUNIR_KR_DL_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/boolean_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/database/semantics/distance_plan.hpp>

namespace urd
{

template<>
struct data<runir::kr::dl::Numerical<runir::kr::dl::CountTag>>
{
    using ConstructorVariant = urd::index_variant<runir::kr::dl::DlArgumentTypes>;
    using Arg = ConstructorVariant;

    urd::index<runir::kr::dl::Numerical<runir::kr::dl::CountTag>> index;
    ConstructorVariant arg;

    data() = default;
    explicit data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg> {}; }
};

/// n_distance(sources, edges, targets): sources/targets are k-column vertices
/// (a concept is k = 1), edges are 2k columns (a role is k = 1).
template<>
struct data<runir::kr::dl::Numerical<runir::kr::dl::DistanceTag>>
{
    using Vertex = urd::index_variant<runir::kr::dl::VertexArgumentTypes>;
    using Edge = urd::index_variant<runir::kr::dl::EdgeArgumentTypes>;

    urd::index<runir::kr::dl::Numerical<runir::kr::dl::DistanceTag>> index;
    Vertex lhs;
    Edge mid;
    Vertex rhs;
    /// Derived on insertion from the argument schemas; not identifying.
    ygg::database::DistancePlan plan;

    data() = default;
    data(Vertex lhs_, Edge mid_, Vertex rhs_) : index(), lhs(std::move(lhs_)), mid(std::move(mid_)), rhs(std::move(rhs_)), plan() {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::lhs, &data::mid, &data::rhs, &data::plan> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::lhs, &data::mid, &data::rhs> {}; }
};

template<>
struct data<runir::kr::dl::Numerical<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Numerical<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                 runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Numerical<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                              runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Numerical<runir::kr::dl::NumericalConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::Numerical<runir::kr::dl::NumericalConstantTag>, ygg::uint_t>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::Numerical<runir::kr::dl::NumericalConstantTag>, ygg::uint_t>;
    using Base::Base;
};

// Binary numerical operators (add/sub/mul/div/min/max): a Numerical over two Numerical operands.
template<runir::kr::dl::NumericalBinaryTag Tag>
struct data<runir::kr::dl::Numerical<Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Numerical<Tag>,
                              runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>,
                              runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Numerical<Tag>,
                                           runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>,
                                           runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace urd

#endif
