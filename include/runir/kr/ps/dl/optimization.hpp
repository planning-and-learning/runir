#ifndef RUNIR_KR_PS_DL_OPTIMIZATION_HPP_
#define RUNIR_KR_PS_DL_OPTIMIZATION_HPP_

#include "runir/kr/dl/optimization.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/optimization.hpp"

#include <stdexcept>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
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
    FeatureOptimizer(runir::kr::dl::ConstructorRepository& repository, const runir::kr::dl::QueryStatistics& statistics) : m_optimizer(repository, statistics)
    {
    }

    template<typename FeatureTag, FamilyRepository Repository>
    RecordView<Repository, ConcreteFeature<Family, FeatureTag>> rewrite(RecordView<Repository, ConcreteFeature<Family, FeatureTag>> view,
                                                                        Repository& repository)
    {
        const auto expression = view.feature();
        const auto optimized = m_optimizer.optimize(expression);
        if (optimized == expression)
            return view;
        auto data = urd::data<ConcreteFeature<Family, FeatureTag>>(optimized.index(), view.symbol());
        return repository.insert(data).first;
    }

private:
    runir::kr::dl::QueryOptimizer m_optimizer;
};

/// Leaf policy for RecordCollector: adds a concrete DL feature's expression to one DL statistics collector.
template<FamilyTag Family>
class FeatureStatisticsCollector
{
public:
    template<typename FeatureTag, FamilyRepository Repository>
    void add(RecordView<Repository, ConcreteFeature<Family, FeatureTag>> view)
    { m_collector.add(view.feature()); }

    [[nodiscard]] const runir::kr::dl::QueryStatisticsCollector& get_collector() const noexcept { return m_collector; }

private:
    runir::kr::dl::QueryStatisticsCollector m_collector;
};

/// Optimizes the queries of any ps record (a base sketch, an ext/icp program, module or rule, an uns classifier, or a
/// single feature) and returns the rewritten record in the same repositories.
template<FamilyRepository Repository, typename T>
RecordView<Repository, T> optimize(RecordView<Repository, T> object, Repository& repository, const runir::kr::dl::QueryStatistics& statistics = {})
{
    using Family = typename Repository::Family;
    if (&object.context() != &repository)
        throw std::invalid_argument("optimize: the object belongs to another repository.");
    auto optimizer = RecordOptimizer<Repository, FeatureOptimizer<Family>>(
        repository,
        FeatureOptimizer<Family>(repository.template get<runir::kr::dl::ConstructorRepository>(), statistics));
    return optimizer.template optimize<T>(object);
}

/// Leaf statistics of every query in a ps record; contexts holds one state evaluation context per sample state. Leaves
/// empty in every sample are reported in warnings.
template<tyr::TaskKind Kind, FamilyRepository Repository, typename T, runir::kr::dl::semantics::StateEvaluationContextConcept<Kind> Context>
runir::kr::dl::QueryStatistics
collect_query_statistics(RecordView<Repository, T> object, const urd::Store<urd::sequence<Context*>>& contexts, ygg::diagnostics::Warnings& warnings)
{
    using Family = typename Repository::Family;
    auto collector = RecordCollector<Repository, FeatureStatisticsCollector<Family>>();
    collector.template add<T>(object);
    return collector.get_leaves().get_collector().template measure<Kind>(contexts, warnings);
}

/// A sampler for the query leaves of a ps record, to measure sample states one at a time with bounded memory.
template<FamilyRepository Repository, typename T>
runir::kr::dl::QueryStatisticsSampler query_statistics_sampler(RecordView<Repository, T> object)
{
    using Family = typename Repository::Family;
    auto collector = RecordCollector<Repository, FeatureStatisticsCollector<Family>>();
    collector.template add<T>(object);
    return collector.get_leaves().get_collector().sampler();
}

}  // namespace runir::kr::ps::dl

#endif
