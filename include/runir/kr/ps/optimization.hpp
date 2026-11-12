#ifndef RUNIR_KR_PS_OPTIMIZATION_HPP_
#define RUNIR_KR_PS_OPTIMIZATION_HPP_

#include "runir/kr/detail/record_key.hpp"
#include "runir/kr/ps/repository.hpp"

#include <boost/mp11.hpp>
#include <urd/elements/children.hpp>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <yggdrasil/formalism/interning.hpp>

/// Language-neutral rewriting and inspection of ps records (sketches, programs, modules, rules, classifiers, features).
/// The feature language plugs in as a leaf policy; the DL policy lives in runir/kr/ps/dl/optimization.hpp.
namespace runir::kr::ps
{

template<FamilyTag Family, typename RepositoryTypes, typename T>
using RecordView = urd::index_view<T, BasicRepository<Family, RepositoryTypes>>;

/// Rebuilds ps records bottom-up. Records the leaf policy can rewrite (leaves.rewrite(view, repository)) are delegated
/// to it; every other record is reinserted with rewritten children, so records whose children are unchanged keep their
/// index. Results are memoized per record.
template<FamilyTag Family, typename RepositoryTypes, typename Leaves>
class RecordOptimizer
{
    template<typename T>
    using View = RecordView<Family, RepositoryTypes, T>;

public:
    RecordOptimizer(BasicRepository<Family, RepositoryTypes>& repository, Leaves leaves) : m_repository(&repository), m_leaves(std::move(leaves)) {}

    template<typename T>
    View<T> optimize(View<T> view)
    {
        const auto key = runir::kr::detail::record_key<RepositoryTypes>(view);
        if (const auto it = m_rewritten.find(key); it != m_rewritten.end())
            return m_repository->view(urd::index<T>(it->second));
        const auto result = rewrite(view);
        m_rewritten.emplace(key, result.index().value);
        return result;
    }

private:
    template<typename T>
    View<T> rewrite(View<T> view)
    {
        if constexpr (requires { m_leaves.rewrite(view, *m_repository); })
            return m_leaves.rewrite(view, *m_repository);
        else
        {
            auto data =
                urd::map_children<RepositoryTypes, T>(view.data(), view.context(), [&]<typename U>(View<U> child) { return optimize<U>(child).index(); });
            return ygg::formalism::insert(*m_repository, data).first;
        }
    }

    BasicRepository<Family, RepositoryTypes>* m_repository;
    Leaves m_leaves;
    urd::Store<urd::map<runir::kr::detail::RecordKey, ygg::uint_t>> m_rewritten;  ///< Rewritten record index by record.
};

/// Visits every record reachable from the added records once and hands those the leaf policy accepts
/// (leaves.add(view)) to it.
template<FamilyTag Family, typename RepositoryTypes, typename Leaves>
class RecordCollector
{
    template<typename T>
    using View = RecordView<Family, RepositoryTypes, T>;

public:
    template<typename T>
    void add(View<T> view)
    {
        if (!m_visited.insert(runir::kr::detail::record_key<RepositoryTypes>(view)).second)
            return;
        if constexpr (requires { m_leaves.add(view); })
            m_leaves.add(view);
        else
            urd::for_each_child<RepositoryTypes, T>(view.data(), view.context(), [&]<typename U>(View<U> child) { add<U>(child); });
    }

    [[nodiscard]] const Leaves& get_leaves() const noexcept { return m_leaves; }

private:
    Leaves m_leaves;
    urd::Store<urd::set<runir::kr::detail::RecordKey>> m_visited;
};

}  // namespace runir::kr::ps

#endif
