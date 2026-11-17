#ifndef RUNIR_KR_DL_RECORD_VIEW_HPP_
#define RUNIR_KR_DL_RECORD_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/numerical_data.hpp"
#include "runir/kr/dl/register_view.hpp"

#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/elements/view.hpp>

/// The getters of a constructor record's views, one per field in field order. Objects and predicates resolve through
/// the planning repository, operands through the view's context.
namespace urd
{

template<typename F, runir::kr::dl::NullaryConceptTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }
};

template<typename F, typename Tag, typename V>
    requires runir::kr::dl::is_atomic_state_tag_v<Tag>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_predicate() const noexcept { return self().template member<&urd::record_of<V>::predicate>(); }
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
};

template<typename F, runir::kr::dl::ConceptBinaryTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::NegationTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }
};

template<typename F, runir::kr::dl::QuantificationTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_role() const noexcept { return self().template member<&urd::record_of<V>::role>(); }

    [[nodiscard]] decltype(auto) get_qualifier() const noexcept { return self().template member<&urd::record_of<V>::qualifier>(); }
};

template<typename F, runir::kr::dl::NumberRestrictionTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_n() const noexcept { return self().template member<&urd::record_of<V>::n>(); }

    [[nodiscard]] decltype(auto) get_role() const noexcept { return self().template member<&urd::record_of<V>::role>(); }
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
};

template<typename F, runir::kr::dl::RoleComparisonTag Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::RoleFillersTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_role() const noexcept { return self().template member<&urd::record_of<V>::role>(); }

    [[nodiscard]] decltype(auto) get_objects() const noexcept { return self().template member<&urd::record_of<V>::objects>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::OneOfTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_objects() const noexcept { return self().template member<&urd::record_of<V>::objects>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::NominalTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_object() const noexcept { return self().template member<&urd::record_of<V>::object>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::RegisterTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_slot() const noexcept { return self().template member<&urd::record_of<V>::slot>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Concept<F, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_argument() const noexcept { return self().template member<&urd::record_of<V>::argument>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Role<F, runir::kr::dl::UniversalTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }
};

template<typename F, typename Tag, typename V>
    requires runir::kr::dl::is_atomic_state_tag_v<Tag>
struct view_mixin<runir::kr::dl::Role<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_predicate() const noexcept { return self().template member<&urd::record_of<V>::predicate>(); }
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
};

template<typename F, runir::kr::dl::RoleBinaryTag Tag, typename V>
struct view_mixin<runir::kr::dl::Role<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }
};

template<typename F, runir::kr::dl::RoleUnaryTag Tag, typename V>
struct view_mixin<runir::kr::dl::Role<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Role<F, runir::kr::dl::RestrictionTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_role() const noexcept { return self().template member<&urd::record_of<V>::role>(); }

    [[nodiscard]] decltype(auto) get_qualifier() const noexcept { return self().template member<&urd::record_of<V>::qualifier>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Role<F, runir::kr::dl::IdentityTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Role<F, runir::kr::dl::RegisterTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_slot() const noexcept { return self().template member<&urd::record_of<V>::slot>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Role<F, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_argument() const noexcept { return self().template member<&urd::record_of<V>::argument>(); }
};

template<typename F, runir::kr::dl::AtomicTag Tag, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_predicate() const noexcept { return self().template member<&urd::record_of<V>::predicate>(); }

    [[nodiscard]] decltype(auto) get_polarity() const noexcept { return self().template member<&urd::record_of<V>::polarity>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, runir::kr::dl::NonemptyTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_argument() const noexcept { return self().template member<&urd::record_of<V>::argument>(); }
};

template<typename F, runir::kr::dl::ComparisonTag Tag, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, runir::kr::dl::BooleanConstantTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_value() const noexcept { return self().template member<&urd::record_of<V>::value>(); }
};

template<typename F, runir::kr::dl::LogicalBinaryTag Tag, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, runir::kr::dl::NotTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_operand() const noexcept { return self().template member<&urd::record_of<V>::operand>(); }
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
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_argument() const noexcept { return self().template member<&urd::record_of<V>::argument>(); }
};

template<typename F, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_value() const noexcept { return self().template member<&urd::record_of<V>::value>(); }
};

template<typename F, runir::kr::dl::NumericalBinaryTag Tag, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_left_operand() const noexcept { return self().template member<&urd::record_of<V>::left_operand>(); }

    [[nodiscard]] decltype(auto) get_right_operand() const noexcept { return self().template member<&urd::record_of<V>::right_operand>(); }
};
}  // namespace urd

#endif
