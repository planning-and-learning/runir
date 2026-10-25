#ifndef RUNIR_KR_PS_OPTIMIZATION_HPP_
#define RUNIR_KR_PS_OPTIMIZATION_HPP_

#include "runir/kr/detail/record_key.hpp"
#include "runir/kr/ps/repository.hpp"

#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/core/children.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/formalism/interning.hpp>

/// Language-neutral rewriting and inspection of ps records (sketches, programs, modules, rules, classifiers, features).
/// The feature language plugs in as a leaf policy; the DL policy lives in runir/kr/ps/dl/optimization.hpp.
namespace runir::kr::ps
{

template<FamilyTag Family, typename RepositoryTypes, typename T>
using RecordView = ygg::View<ygg::Index<T>, BasicRepository<Family, RepositoryTypes>>;

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
            return ygg::make_view(ygg::Index<T>(it->second), *m_repository);
        const auto result = rewrite(view);
        m_rewritten.emplace(key, result.get_index().get_value());
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
            auto data = ygg::map_children<RepositoryTypes>(view, [&]<typename U>(View<U> child) { return optimize<U>(child); });
            return ygg::formalism::insert(*m_repository, data).first;
        }
    }

    BasicRepository<Family, RepositoryTypes>* m_repository;
    Leaves m_leaves;
    ygg::UnorderedMap<runir::kr::detail::RecordKey, ygg::uint_t> m_rewritten;  ///< Rewritten record index by record.
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
            ygg::for_each_child<RepositoryTypes>(view, [&]<typename U>(View<U> child) { add<U>(child); });
    }

    [[nodiscard]] const Leaves& get_leaves() const noexcept { return m_leaves; }

private:
    Leaves m_leaves;
    ygg::UnorderedSet<runir::kr::detail::RecordKey> m_visited;
};

}  // namespace runir::kr::ps

#endif
