#ifndef RUNIR_KR_DL_RECORD_VIEW_HPP_
#define RUNIR_KR_DL_RECORD_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/numerical_data.hpp"
#include "runir/kr/dl/register_view.hpp"

#include <cstddef>
#include <type_traits>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/elements/view.hpp>

/// The getters of a constructor record's views, one per field in field order. Objects and predicates resolve through
/// the planning repository, operands through the view's context.
namespace runir::kr::dl::semantics
{
/// The syntactic complexity of a constructor record of the semantic family (defined in syntactic_complexity.hpp),
/// declared before the records' views, whose functions() list it.
template<typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, Tag>, C>& view);

template<typename Tag, typename C>
    requires runir::kr::dl::RoleConstructorTag<Tag>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::Role<runir::kr::dl::SemanticFamily, Tag>, C>& view);

template<typename Tag, typename C>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, Tag>, C>& view);

template<typename Tag, typename C>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, Tag>, C>& view);

/// Whether the view V has a syntactic complexity: the view of a constructor record of the semantic family.
template<typename V>
concept Measured = requires(const V& view) { runir::kr::dl::semantics::syntactic_complexity(view); };
}  // namespace runir::kr::dl::semantics

namespace urd
{

template<typename F, runir::kr::dl::NullaryConceptTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename Tag, typename V>
    requires runir::kr::dl::is_atomic_state_tag_v<Tag>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_predicate() const noexcept { return self().template member<&urd::record_of<V>::predicate>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename Tag, typename V>
    requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_predicate() const noexcept { return self().template member<&urd::record_of<V>::predicate>(); }

    [[nodiscard]] decltype(auto) get_polarity() const noexcept { return self().template member<&urd::record_of<V>::polarity>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::ConceptBinaryTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::NegationTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::QuantificationTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_role() const noexcept { return self().template member<&urd::record_of<V>::role>(); }

    [[nodiscard]] decltype(auto) get_qualifier() const noexcept { return self().template member<&urd::record_of<V>::qualifier>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::NumberRestrictionTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_n() const noexcept { return self().template member<&urd::record_of<V>::n>(); }

    [[nodiscard]] decltype(auto) get_role() const noexcept { return self().template member<&urd::record_of<V>::role>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::QualifiedNumberRestrictionTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_n() const noexcept { return self().template member<&urd::record_of<V>::n>(); }

    [[nodiscard]] decltype(auto) get_role() const noexcept { return self().template member<&urd::record_of<V>::role>(); }

    [[nodiscard]] decltype(auto) get_qualifier() const noexcept { return self().template member<&urd::record_of<V>::qualifier>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::RoleComparisonTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::RoleFillersTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_role() const noexcept { return self().template member<&urd::record_of<V>::role>(); }

    [[nodiscard]] decltype(auto) get_objects() const noexcept { return self().template member<&urd::record_of<V>::objects>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::OneOfTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_objects() const noexcept { return self().template member<&urd::record_of<V>::objects>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::NominalTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_object() const noexcept { return self().template member<&urd::record_of<V>::object>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::RegisterTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_slot() const noexcept { return self().template member<&urd::record_of<V>::slot>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_argument() const noexcept { return self().template member<&urd::record_of<V>::argument>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Role<F, runir::kr::dl::UniversalTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename Tag, typename V>
    requires runir::kr::dl::is_atomic_state_tag_v<Tag>
struct view_mixin<runir::kr::dl::Role<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_predicate() const noexcept { return self().template member<&urd::record_of<V>::predicate>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename Tag, typename V>
    requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
struct view_mixin<runir::kr::dl::Role<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_predicate() const noexcept { return self().template member<&urd::record_of<V>::predicate>(); }

    [[nodiscard]] decltype(auto) get_polarity() const noexcept { return self().template member<&urd::record_of<V>::polarity>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::RoleBinaryTag Tag, typename V>
struct view_mixin<runir::kr::dl::Role<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::RoleUnaryTag Tag, typename V>
struct view_mixin<runir::kr::dl::Role<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Role<F, runir::kr::dl::RestrictionTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_role() const noexcept { return self().template member<&urd::record_of<V>::role>(); }

    [[nodiscard]] decltype(auto) get_qualifier() const noexcept { return self().template member<&urd::record_of<V>::qualifier>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Role<F, runir::kr::dl::IdentityTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Role<F, runir::kr::dl::RegisterTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_slot() const noexcept { return self().template member<&urd::record_of<V>::slot>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Role<F, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_argument() const noexcept { return self().template member<&urd::record_of<V>::argument>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::AtomicTag Tag, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_predicate() const noexcept { return self().template member<&urd::record_of<V>::predicate>(); }

    [[nodiscard]] decltype(auto) get_polarity() const noexcept { return self().template member<&urd::record_of<V>::polarity>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, runir::kr::dl::NonemptyTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_argument() const noexcept { return self().template member<&urd::record_of<V>::argument>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::ComparisonTag Tag, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, runir::kr::dl::BooleanConstantTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_value() const noexcept { return self().template member<&urd::record_of<V>::value>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::LogicalBinaryTag Tag, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, runir::kr::dl::NotTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, runir::kr::dl::ValueTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, runir::kr::dl::DistanceTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_sources() const noexcept { return self().template member<&urd::record_of<V>::sources>(); }

    [[nodiscard]] decltype(auto) get_edges() const noexcept { return self().template member<&urd::record_of<V>::edges>(); }

    [[nodiscard]] decltype(auto) get_targets() const noexcept { return self().template member<&urd::record_of<V>::targets>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename V>
struct view_mixin<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_sources() const noexcept { return self().template member<&urd::record_of<V>::sources>(); }

    [[nodiscard]] decltype(auto) get_edges() const noexcept { return self().template member<&urd::record_of<V>::edges>(); }

    [[nodiscard]] decltype(auto) get_targets() const noexcept { return self().template member<&urd::record_of<V>::targets>(); }

    [[nodiscard]] decltype(auto) get_plan() const noexcept { return self().template member<&urd::record_of<V>::plan>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_argument() const noexcept { return self().template member<&urd::record_of<V>::argument>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_value() const noexcept { return self().template member<&urd::record_of<V>::value>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};

template<typename F, runir::kr::dl::NumericalBinaryTag Tag, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }

    static constexpr auto functions() noexcept
    {
        if constexpr (runir::kr::dl::semantics::Measured<V>)
            return urd::Functions<
                urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
        else
            return urd::Functions<> {};
    }
};
}  // namespace urd

#endif
