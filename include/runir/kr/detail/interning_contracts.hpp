#ifndef RUNIR_KR_DETAIL_INTERNING_CONTRACTS_HPP_
#define RUNIR_KR_DETAIL_INTERNING_CONTRACTS_HPP_

#include <concepts>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::detail
{

/// Compile-time contracts of a repository, checked in the library source that instantiates it: there they cost
/// almost nothing, while a test checking them instantiates every repository again (template rule 17).
template<typename Repository, typename T>
concept InternsData = requires(Repository& repository, ygg::Data<T>& data) {
    { ygg::formalism::insert(repository, data) } -> std::same_as<std::pair<ygg::View<ygg::Index<T>, Repository>, bool>>;
};

template<typename Repository, typename T>
consteval bool rejects_symbol()
{
    return !ygg::formalism::SupportsSymbol<Repository, T> && !InternsData<Repository, T>
           && !requires(Repository& repository, ygg::Data<T>& data) { repository.insert(data); }
           && !requires(const Repository& repository, const ygg::Data<T>& data) { repository.find(data); }
           && !requires(const Repository& repository, ygg::Index<T> index) { repository[index]; }
           && !requires(const Repository& repository) { repository.template size<T>(); };
}

}  // namespace runir::kr::detail

#endif
