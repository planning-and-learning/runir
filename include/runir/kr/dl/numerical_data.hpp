#ifndef RUNIR_KR_DL_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/boolean_data.hpp"
#include <yggdrasil/containers/variant.hpp>

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/database/semantics/distance.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Numerical<Family, runir::kr::dl::CountTag>>
{
    using ConstructorVariant = ::ygg::IndexVariant<DlArgumentTypes<Family>>;
    using Arg = ConstructorVariant;

    Index<runir::kr::dl::Numerical<Family, runir::kr::dl::CountTag>> index;
    ConstructorVariant arg;

    Data() = default;
    explicit Data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}
    template<typename C>
    using ViewVariant =
        std::variant<::ygg::View<Index<DlConcept<Family>>, C>, ::ygg::View<Index<DlRole<Family>>, C>, ::ygg::View<Index<runir::kr::dl::Query<Family>>, C>>;
    template<typename C>
    explicit Data(ViewVariant<C> arg_) :
        index(),
        arg(std::visit([](const auto& view) -> ConstructorVariant { return ConstructorVariant(view.get_index()); }, arg_))
    {
    }

    auto cista_members() noexcept { return std::tie(index, arg); }
    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

/// n_distance(sources, edges, targets): sources/targets are k-column vertices
/// (a concept is k = 1), edges are 2k columns (a role is k = 1).
template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Numerical<Family, runir::kr::dl::DistanceTag>>
{
    using Vertex = ::ygg::IndexVariant<VertexArgumentTypes<Family>>;
    using Edge = ::ygg::IndexVariant<EdgeArgumentTypes<Family>>;

    Index<runir::kr::dl::Numerical<Family, runir::kr::dl::DistanceTag>> index;
    Vertex lhs;
    Edge mid;
    Vertex rhs;
    /// Derived on insertion from the argument schemas; not identifying.
    database::DistancePlan<runir::kr::dl::QueryValues> plan;

    Data() = default;
    Data(Vertex lhs_, Edge mid_, Vertex rhs_) : index(), lhs(std::move(lhs_)), mid(std::move(mid_)), rhs(std::move(rhs_)), plan() {}
    template<typename C>
    Data(const ::ygg::ViewVariant<Vertex, C>& lhs_, const ::ygg::ViewVariant<Edge, C>& mid_, const ::ygg::ViewVariant<Vertex, C>& rhs_) :
        index(),
        lhs(std::visit([](const auto& view) -> Vertex { return Vertex(view.get_index()); }, lhs_)),
        mid(std::visit([](const auto& view) -> Edge { return Edge(view.get_index()); }, mid_)),
        rhs(std::visit([](const auto& view) -> Vertex { return Vertex(view.get_index()); }, rhs_)),
        plan()
    {
    }

    auto cista_members() noexcept { return std::tie(index, lhs, mid, rhs, plan); }
    auto cista_members() const noexcept { return std::tie(index, lhs, mid, rhs, plan); }
    auto identifying_members() const noexcept { return std::tie(lhs, mid, rhs); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<>
struct Data<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                 runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                              runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Numerical<Family, runir::kr::dl::NumericalConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>;
    using Base::Base;
};

// Binary numerical operators (add/sub/mul/div/min/max): a Numerical over two Numerical operands.
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalBinaryTag Tag>
struct Data<runir::kr::dl::Numerical<Family, Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Numerical<Family, Tag>,
                              runir::kr::dl::Constructor<Family, runir::kr::dl::NumericalTag>,
                              runir::kr::dl::Constructor<Family, runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Numerical<Family, Tag>,
                                           runir::kr::dl::Constructor<Family, runir::kr::dl::NumericalTag>,
                                           runir::kr::dl::Constructor<Family, runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
