#ifndef RUNIR_KR_PS_BASE_DECLARATIONS_HPP_
#define RUNIR_KR_PS_BASE_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <boost/mp11.hpp>
#include <memory>
#include <string_view>
#include <urd/elements/flat.hpp>

namespace runir::kr::ps::base
{

// Sketch

struct Sketch
{
    static constexpr auto name = "Sketch";
};

using FeatureTypes = runir::kr::ps::PsFeatureTypes<runir::kr::BaseFamilyTag>;
using ConditionTypes = runir::kr::ps::PsConditionTypes<runir::kr::BaseFamilyTag>;
using EffectTypes = runir::kr::ps::PsEffectTypes<runir::kr::BaseFamilyTag>;
using SketchTypes = boost::mp11::mp_list<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>, runir::kr::ps::base::Sketch>;
using RepositoryTypes = boost::mp11::mp_append<runir::kr::ps::PsCoreTypes<runir::kr::BaseFamilyTag>, SketchTypes>;
using Repository = runir::kr::ps::BasicRepository<runir::kr::BaseFamilyTag, RepositoryTypes>;
using RepositoryPtr = std::shared_ptr<Repository>;

using SketchView = urd::index_view<runir::kr::ps::base::Sketch, Repository>;
using RuleView = urd::index_view<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>, Repository>;

/// The labels of its proof graphs' vertices and edges (records: data<...>, the graph's elements).
template<tyr::TaskKind Kind>
struct SketchProofVertexLabel
{
    static constexpr std::string_view name = urd::joined_name<"SketchProofVertexLabel", Kind::name>;
};

struct SketchProofEdgeLabel
{
    static constexpr std::string_view name = "SketchProofEdgeLabel";
};
}  // namespace runir::kr::ps::base

template<>
struct runir::kr::ps::FamilyTypes<runir::kr::BaseFamilyTag>
{
    using Repository = runir::kr::ps::base::Repository;
};

namespace runir::kr::ps::base::dl
{

using BooleanFeatureView = urd::index_view<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>, Repository>;
using NumericalFeatureView = urd::index_view<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>, Repository>;

}  // namespace runir::kr::ps::base::dl

#endif
