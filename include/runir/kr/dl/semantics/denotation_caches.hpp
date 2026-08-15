#ifndef RUNIR_KR_DL_SEMANTICS_DENOTATION_CACHES_HPP_
#define RUNIR_KR_DL_SEMANTICS_DENOTATION_CACHES_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/semantics/equal_to.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::kr::dl::semantics
{

/// Constructor denotations for one fixed evaluation state.
/// The referenced constructor and denotation repositories must outlive the cache entries.
/// Ext denotations also depend on mutable registers and arguments, which are not part of the key.
template<FamilyTag Family>
    requires(!std::same_as<Family, runir::kr::ExtFamilyTag>)
struct DenotationCaches
{
    template<CategoryTag Category>
    using Cache = ygg::UnorderedMap<FamilyConstructorView<Family, Category>, DenotationView<Category>>;

    std::tuple<Cache<ConceptTag>, Cache<RoleTag>, Cache<BooleanTag>, Cache<NumericalTag>> values;

    template<CategoryTag Category>
    auto& get() noexcept
    {
        return std::get<Cache<Category>>(values);
    }

    template<CategoryTag Category>
    const auto& get() const noexcept
    {
        return std::get<Cache<Category>>(values);
    }

    void clear() noexcept
    {
        std::apply([](auto&... caches) { (caches.clear(), ...); }, values);
    }
};

}  // namespace runir::kr::dl::semantics

#endif
