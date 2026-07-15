#ifndef RUNIR_KR_DL_SEMANTICS_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_DL_SEMANTICS_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/dl/semantics/views.hpp"

#include <concepts>
#include <cstddef>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::semantics
{

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C> view);

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::FamilyRole<Family, Tag>>, C> view);

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C> view);

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C> view);

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C> view)
{
    return ygg::visit([](auto child) { return syntactic_complexity(child); }, view.get_variant());
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C> view)
{
    if constexpr (requires { view.get_arg(); })
        return 1 + syntactic_complexity(view.get_arg());
    else if constexpr (requires {
                           view.get_lhs();
                           view.get_rhs();
                       })
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else if constexpr (requires {
                           view.get_role();
                           view.get_concept();
                       })
        return 1 + syntactic_complexity(view.get_role()) + syntactic_complexity(view.get_concept());
    else if constexpr (requires { view.get_role(); })
        return 1 + syntactic_complexity(view.get_role());
    else
        return 1;
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::FamilyRole<Family, Tag>>, C> view)
{
    if constexpr (requires { view.get_arg(); })
        return 1 + syntactic_complexity(view.get_arg());
    else if constexpr (requires {
                           view.get_lhs();
                           view.get_rhs();
                       })
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else
        return 1;
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return 1 + ygg::visit([](auto child) { return syntactic_complexity(child); }, view.get_arg());
    else if constexpr (requires { view.get_arg(); })
        return 1 + syntactic_complexity(view.get_arg());
    else if constexpr (requires {
                           view.get_lhs();
                           view.get_rhs();
                       })
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else
        return 1;
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return 1 + ygg::visit([](auto child) { return syntactic_complexity(child); }, view.get_arg());
    else if constexpr (requires {
                           view.get_lhs();
                           view.get_mid();
                           view.get_rhs();
                       })
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_mid()) + syntactic_complexity(view.get_rhs());
    else if constexpr (requires {
                           view.get_lhs();
                           view.get_rhs();
                       })
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else
        return 1;
}

}  // namespace runir::kr::dl::semantics

#endif
