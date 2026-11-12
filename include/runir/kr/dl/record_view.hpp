#ifndef RUNIR_KR_DL_RECORD_VIEW_HPP_
#define RUNIR_KR_DL_RECORD_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/numerical_data.hpp"
#include "runir/kr/dl/register_view.hpp"

#include <boost/mp11.hpp>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/elements/view.hpp>

namespace runir::kr::dl
{

/// The accessors of a view V of a constructor record (Concept/Role/Boolean/Numerical<Family, Tag>): one per member of
/// its record, so a view has exactly the accessors of what its record holds. Objects and predicates resolve through
/// the planning repository, operands through the view's context. Precondition of every accessor: the record's operands
/// are assigned and its indices belong to the context's repositories, as an interned record's are. Resolving throws for a
/// record that breaks it (std::bad_variant_access, std::out_of_range), which ends the program in these noexcept accessors.
template<typename V>
struct RecordViewMixin
{
private:
    /// Only a view and the urd::view_mixin it derives from make one.
    RecordViewMixin() = default;
    friend V;
    template<typename, typename>
    friend struct urd::view_mixin;

    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    // NOLINTNEXTLINE(bugprone-exception-escape): resolving throws only for a record that breaks the precondition stated at the mixin
    [[nodiscard]] auto get_predicate() const noexcept
        requires requires(const urd::record_of<V>& record) { record.predicate; }
    { return urd::resolve(self().data().predicate, self().context().get_planning_repository()); }

    [[nodiscard]] auto get_polarity() const noexcept
        requires requires(const urd::record_of<V>& record) { record.polarity; }
    { return self().data().polarity; }

    // NOLINTNEXTLINE(bugprone-exception-escape): resolving throws only for a record that breaks the precondition stated at the mixin
    [[nodiscard]] auto get_object() const noexcept
        requires requires(const urd::record_of<V>& record) { record.object; }
    { return urd::resolve(self().data().object, self().context().get_planning_repository()); }

    [[nodiscard]] auto get_objects() const noexcept
        requires requires(const urd::record_of<V>& record) { record.objects; }
    { return urd::resolve(self().data().objects, self().context().get_planning_repository()); }

    [[nodiscard]] auto get_n() const noexcept
        requires requires(const urd::record_of<V>& record) { record.n; }
    { return self().data().n; }

    // NOLINTNEXTLINE(bugprone-exception-escape): resolving throws only for a record that breaks the precondition stated at the mixin
    [[nodiscard]] auto get_role() const noexcept
        requires requires(const urd::record_of<V>& record) { record.role; }
    { return self().template member<&urd::record_of<V>::role>(); }

    // NOLINTNEXTLINE(bugprone-exception-escape): resolving throws only for a record that breaks the precondition stated at the mixin
    [[nodiscard]] auto get_concept() const noexcept
        requires requires(const urd::record_of<V>& record) { record.concept_; }
    { return self().template member<&urd::record_of<V>::concept_>(); }

    // NOLINTNEXTLINE(bugprone-exception-escape): resolving throws only for a record that breaks the precondition stated at the mixin
    [[nodiscard]] auto get_arg() const noexcept
        requires requires(const urd::record_of<V>& record) { record.arg; }
    { return self().template member<&urd::record_of<V>::arg>(); }

    // NOLINTNEXTLINE(bugprone-exception-escape): resolving throws only for a record that breaks the precondition stated at the mixin
    [[nodiscard]] auto get_lhs() const noexcept
        requires requires(const urd::record_of<V>& record) { record.lhs; }
    { return self().template member<&urd::record_of<V>::lhs>(); }

    // NOLINTNEXTLINE(bugprone-exception-escape): resolving throws only for a record that breaks the precondition stated at the mixin
    [[nodiscard]] auto get_rhs() const noexcept
        requires requires(const urd::record_of<V>& record) { record.rhs; }
    { return self().template member<&urd::record_of<V>::rhs>(); }

    [[nodiscard]] auto get_register() const noexcept
        requires boost::mp11::mp_similar<typename urd::record_of<V>::Referenced, Register<ConceptTag>>::value
    { return self().template member<&urd::record_of<V>::reference>(); }

    [[nodiscard]] auto get_argument() const noexcept
        requires boost::mp11::mp_similar<typename urd::record_of<V>::Referenced, Argument<ConceptTag>>::value
    { return self().template member<&urd::record_of<V>::reference>(); }

    [[nodiscard]] auto get_value() const noexcept
        requires requires(const urd::record_of<V>& record) { record.identifier; }
    { return self().data().identifier; }
};

}  // namespace runir::kr::dl

namespace urd
{
template<typename F, typename Tag, typename V>
struct view_mixin<runir::kr::dl::Concept<F, Tag>, V> : runir::kr::dl::RecordViewMixin<V>
{
};

template<typename F, typename Tag, typename V>
struct view_mixin<runir::kr::dl::Role<F, Tag>, V> : runir::kr::dl::RecordViewMixin<V>
{
};

template<typename F, typename Tag, typename V>
struct view_mixin<runir::kr::dl::Boolean<F, Tag>, V> : runir::kr::dl::RecordViewMixin<V>
{
};

template<typename F, typename Tag, typename V>
struct view_mixin<runir::kr::dl::Numerical<F, Tag>, V> : runir::kr::dl::RecordViewMixin<V>
{
};
}  // namespace urd

#endif
