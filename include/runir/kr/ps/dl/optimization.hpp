#ifndef RUNIR_KR_PS_DL_OPTIMIZATION_HPP_
#define RUNIR_KR_PS_DL_OPTIMIZATION_HPP_

#include "runir/kr/dl/optimization.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/optimization.hpp"

#include <stdexcept>
#include <vector>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::ps::dl
{

/// Leaf policy for RecordOptimizer: optimizes a concrete DL feature's expression and keeps its symbol. One DL optimizer
/// serves the whole record, so features share rewritten subexpressions.
template<FamilyTag Family>
class FeatureOptimizer
{
public:
    FeatureOptimizer(runir::kr::dl::ConstructorRepositoryFor& repository, const runir::kr::dl::QueryStatistics& statistics) :
        m_optimizer(repository, statistics)
    {
    }

    template<typename FeatureTag, typename RepositoryTypes>
    RecordView<Family, RepositoryTypes, ConcreteFeature<Family, DlTag, FeatureTag>>
    rewrite(RecordView<Family, RepositoryTypes, ConcreteFeature<Family, DlTag, FeatureTag>> view, BasicRepository<Family, RepositoryTypes>& repository)
    {
        const auto expression = view.get_expression();
        const auto optimized = m_optimizer.optimize(expression);
        if (optimized == expression)
            return view;
        auto data = ygg::Data<ConcreteFeature<Family, DlTag, FeatureTag>>(optimized, view.get_symbol());
        return ygg::formalism::insert(repository, data).first;
    }

private:
    runir::kr::dl::QueryOptimizer m_optimizer;
};

/// Leaf policy for RecordCollector: adds a concrete DL feature's expression to one DL statistics collector.
template<FamilyTag Family>
class FeatureStatisticsCollector
{
public:
    template<typename FeatureTag, typename RepositoryTypes>
    void add(RecordView<Family, RepositoryTypes, ConcreteFeature<Family, DlTag, FeatureTag>> view)
    {
        m_collector.add(view.get_expression());
    }

    const runir::kr::dl::QueryStatisticsCollector& get_collector() const noexcept { return m_collector; }

private:
    runir::kr::dl::QueryStatisticsCollector m_collector;
};

/// Optimizes the queries of any ps record (a base sketch, an ext/icp program, module or rule, an uns classifier, or a
/// single feature) and returns the rewritten record in the same repositories.
template<FamilyTag Family, typename T, typename RepositoryTypes>
RecordView<Family, RepositoryTypes, T> optimize(RecordView<Family, RepositoryTypes, T> object,
                                                BasicRepository<Family, RepositoryTypes>& repository,
                                                const runir::kr::dl::QueryStatistics& statistics = {})
{
    if (&object.get_context() != &repository)
        throw std::invalid_argument("optimize: the object belongs to another repository.");
    auto optimizer =
        RecordOptimizer<Family, RepositoryTypes, FeatureOptimizer<Family>>(repository, FeatureOptimizer<Family>(repository.get_dl_repository(), statistics));
    return optimizer.template optimize<T>(object);
}

/// Leaf statistics of every query in a ps record; contexts holds one state evaluation context per sample state. Leaves
/// empty in every sample are reported in warnings.
template<tyr::TaskKind Kind, FamilyTag Family, typename T, typename RepositoryTypes, runir::kr::dl::semantics::StateEvaluationContextConcept<Kind> Context>
runir::kr::dl::QueryStatistics
collect_query_statistics(RecordView<Family, RepositoryTypes, T> object, const std::vector<Context*>& contexts, ygg::diagnostics::Warnings& warnings)
{
    auto collector = RecordCollector<Family, RepositoryTypes, FeatureStatisticsCollector<Family>>();
    collector.template add<T>(object);
    return collector.get_leaves().get_collector().template measure<Kind>(contexts, warnings);
}

/// A sampler for the query leaves of a ps record, to measure sample states one at a time with bounded memory.
template<FamilyTag Family, typename T, typename RepositoryTypes>
runir::kr::dl::QueryStatisticsSampler query_statistics_sampler(RecordView<Family, RepositoryTypes, T> object)
{
    auto collector = RecordCollector<Family, RepositoryTypes, FeatureStatisticsCollector<Family>>();
    collector.template add<T>(object);
    return collector.get_leaves().get_collector().sampler();
}

}  // namespace runir::kr::ps::dl

#endif
