#ifndef RUNIR_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/semantics/boolean_view.hpp"
#include "runir/semantics/concept_view.hpp"
#include "runir/semantics/constructor_data.hpp"
#include "runir/semantics/numerical_view.hpp"
#include "runir/semantics/role_view.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <variant>

namespace tyr
{

namespace runir_detail
{
template<runir::CategoryTag Category, typename C>
struct ConstructorViewVariant;

template<typename C>
struct ConstructorViewVariant<runir::ConceptTag, C>
{
    using Type = std::variant<View<Index<runir::Concept<runir::BotTag>>, C>,
                              View<Index<runir::Concept<runir::TopTag>>, C>,
                              View<Index<runir::Concept<runir::AtomicStateTag<formalism::StaticTag>>>, C>,
                              View<Index<runir::Concept<runir::AtomicStateTag<formalism::FluentTag>>>, C>,
                              View<Index<runir::Concept<runir::AtomicStateTag<formalism::DerivedTag>>>, C>,
                              View<Index<runir::Concept<runir::AtomicGoalTag<formalism::StaticTag>>>, C>,
                              View<Index<runir::Concept<runir::AtomicGoalTag<formalism::FluentTag>>>, C>,
                              View<Index<runir::Concept<runir::AtomicGoalTag<formalism::DerivedTag>>>, C>,
                              View<Index<runir::Concept<runir::IntersectionTag>>, C>,
                              View<Index<runir::Concept<runir::UnionTag>>, C>,
                              View<Index<runir::Concept<runir::NegationTag>>, C>,
                              View<Index<runir::Concept<runir::ValueRestrictionTag>>, C>,
                              View<Index<runir::Concept<runir::ExistentialQuantificationTag>>, C>,
                              View<Index<runir::Concept<runir::RoleValueMapContainmentTag>>, C>,
                              View<Index<runir::Concept<runir::RoleValueMapEqualityTag>>, C>,
                              View<Index<runir::Concept<runir::NominalTag>>, C>>;
};

template<typename C>
struct ConstructorViewVariant<runir::RoleTag, C>
{
    using Type = std::variant<View<Index<runir::Role<runir::UniversalTag>>, C>,
                              View<Index<runir::Role<runir::AtomicStateTag<formalism::StaticTag>>>, C>,
                              View<Index<runir::Role<runir::AtomicStateTag<formalism::FluentTag>>>, C>,
                              View<Index<runir::Role<runir::AtomicStateTag<formalism::DerivedTag>>>, C>,
                              View<Index<runir::Role<runir::AtomicGoalTag<formalism::StaticTag>>>, C>,
                              View<Index<runir::Role<runir::AtomicGoalTag<formalism::FluentTag>>>, C>,
                              View<Index<runir::Role<runir::AtomicGoalTag<formalism::DerivedTag>>>, C>,
                              View<Index<runir::Role<runir::IntersectionTag>>, C>,
                              View<Index<runir::Role<runir::UnionTag>>, C>,
                              View<Index<runir::Role<runir::ComplementTag>>, C>,
                              View<Index<runir::Role<runir::InverseTag>>, C>,
                              View<Index<runir::Role<runir::CompositionTag>>, C>,
                              View<Index<runir::Role<runir::TransitiveClosureTag>>, C>,
                              View<Index<runir::Role<runir::ReflexiveTransitiveClosureTag>>, C>,
                              View<Index<runir::Role<runir::RestrictionTag>>, C>,
                              View<Index<runir::Role<runir::IdentityTag>>, C>>;
};

template<typename C>
struct ConstructorViewVariant<runir::BooleanTag, C>
{
    using Type = std::variant<View<Index<runir::Boolean<runir::AtomicStateTag<formalism::StaticTag>>>, C>,
                              View<Index<runir::Boolean<runir::AtomicStateTag<formalism::FluentTag>>>, C>,
                              View<Index<runir::Boolean<runir::AtomicStateTag<formalism::DerivedTag>>>, C>,
                              View<Index<runir::Boolean<runir::NonemptyTag>>, C>>;
};

template<typename C>
struct ConstructorViewVariant<runir::NumericalTag, C>
{
    using Type = std::variant<View<Index<runir::Numerical<runir::CountTag>>, C>, View<Index<runir::Numerical<runir::DistanceTag>>, C>>;
};
}

template<runir::CategoryTag Category, typename C>
class View<Index<runir::Constructor<Category>>, C>
{
private:
    const C* m_context;
    Index<runir::Constructor<Category>> m_handle;

public:
    using VariantView = typename runir_detail::ConstructorViewVariant<Category, C>::Type;

    View(Index<runir::Constructor<Category>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto get_variant() const noexcept
    {
        return std::visit([&](const auto& arg) -> VariantView { return VariantView(make_view(arg, *m_context)); }, get_data().value);
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
