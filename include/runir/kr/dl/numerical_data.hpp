#ifndef RUNIR_KR_DL_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/boolean_data.hpp"

#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>
#include <tyr/datalog/lifted/database/semantics/distance_plan.hpp>

namespace urd
{

template<typename F>
struct data<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>, typename F::Argument>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>, typename F::Argument>;
    using Base::Base;
};

/// n_distance(sources, edges, targets).
template<typename F>
struct data<runir::kr::dl::Numerical<F, runir::kr::dl::DistanceTag>> :
    runir::kr::dl::TernaryData<runir::kr::dl::Numerical<F, runir::kr::dl::DistanceTag>, typename F::Vertex, typename F::Edge>
{
    using Base = runir::kr::dl::TernaryData<runir::kr::dl::Numerical<F, runir::kr::dl::DistanceTag>, typename F::Vertex, typename F::Edge>;
    using Base::Base;
};

/// A semantic distance also holds its plan: sources/targets are k-column vertices, edges 2k columns.
template<>
struct data<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>> :
    runir::kr::dl::TernaryData<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>,
                               runir::kr::dl::SemanticFamily::Vertex,
                               runir::kr::dl::SemanticFamily::Edge>
{
    using Base = runir::kr::dl::TernaryData<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>,
                                            runir::kr::dl::SemanticFamily::Vertex,
                                            runir::kr::dl::SemanticFamily::Edge>;
    using Base::Base;

    /// Derived on insertion from the argument schemas; not identifying.
    tyr::datalog::database::DistancePlan plan {};

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::lhs, &data::mid, &data::rhs, &data::plan> {}; }
};

template<typename F>
struct data<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                 runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                              runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

template<typename F>
struct data<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>;
    using Base::Base;
};

// Binary numerical operators (add/sub/mul/div/min/max): a Numerical over two Numerical operands.
template<typename F, runir::kr::dl::NumericalBinaryTag Tag>
struct data<runir::kr::dl::Numerical<F, Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Numerical<F, Tag>,
                              runir::kr::dl::operand_t<F, runir::kr::dl::NumericalTag>,
                              runir::kr::dl::operand_t<F, runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Numerical<F, Tag>,
                                           runir::kr::dl::operand_t<F, runir::kr::dl::NumericalTag>,
                                           runir::kr::dl::operand_t<F, runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace urd

#endif
