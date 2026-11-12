#ifndef RUNIR_KR_DETAIL_INTERNING_CONTRACTS_HPP_
#define RUNIR_KR_DETAIL_INTERNING_CONTRACTS_HPP_

#include <concepts>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/formalism/declarations.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::detail
{

/// Compile-time contracts of a repository, checked in the library source that instantiates it: there they cost
/// almost nothing, while a test checking them instantiates every repository again (template rule 17).
template<typename Repository, typename T>
concept InternsData = requires(Repository& repository, urd::data<T>& data) {
    { ygg::formalism::insert(repository, data) } -> std::same_as<std::pair<urd::index_view<T, Repository>, bool>>;
};

template<typename Repository, typename T>
consteval bool rejects_symbol()
{
    return !ygg::formalism::SupportsSymbol<Repository, T> && !InternsData<Repository, T>
           && !requires(Repository& repository, urd::data<T>& data) { repository.insert(data); }
           && !requires(const Repository& repository, const urd::data<T>& data) { repository.find(data); }
           && !requires(const Repository& repository, urd::index<T> index) { repository[index]; }
           && !requires(const Repository& repository) { repository.template size<T>(); };
}

}  // namespace runir::kr::detail

#endif
