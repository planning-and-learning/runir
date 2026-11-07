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
/// the planning repository, operands through the view's context.
template<typename V>
struct RecordViewMixin
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_predicate() const noexcept
        requires requires(const urd::record_of<V>& record) { record.predicate; }
    { return urd::resolve(self().data().predicate, self().context().get_planning_repository()); }

    auto get_polarity() const noexcept
        requires requires(const urd::record_of<V>& record) { record.polarity; }
    { return self().data().polarity; }

    auto get_object() const noexcept
        requires requires(const urd::record_of<V>& record) { record.object; }
    { return urd::resolve(self().data().object, self().context().get_planning_repository()); }

    auto get_objects() const noexcept
        requires requires(const urd::record_of<V>& record) { record.objects; }
    { return urd::resolve(self().data().objects, self().context().get_planning_repository()); }

    auto get_n() const noexcept
        requires requires(const urd::record_of<V>& record) { record.n; }
    { return self().data().n; }

    auto get_role() const noexcept
        requires requires(const urd::record_of<V>& record) { record.role; }
    { return self().template get<&urd::record_of<V>::role>(); }

    auto get_concept() const noexcept
        requires requires(const urd::record_of<V>& record) { record.concept_; }
    { return self().template get<&urd::record_of<V>::concept_>(); }

    auto get_arg() const noexcept
        requires requires(const urd::record_of<V>& record) { record.arg; }
    { return self().template get<&urd::record_of<V>::arg>(); }

    auto get_lhs() const noexcept
        requires requires(const urd::record_of<V>& record) { record.lhs; }
    { return self().template get<&urd::record_of<V>::lhs>(); }

    auto get_mid() const noexcept
        requires requires(const urd::record_of<V>& record) { record.mid; }
    { return self().template get<&urd::record_of<V>::mid>(); }

    auto get_rhs() const noexcept
        requires requires(const urd::record_of<V>& record) { record.rhs; }
    { return self().template get<&urd::record_of<V>::rhs>(); }

    auto get_register() const noexcept
        requires boost::mp11::mp_similar<typename urd::record_of<V>::Referenced, Register<ConceptTag>>::value
    { return self().template get<&urd::record_of<V>::reference>(); }

    auto get_argument() const noexcept
        requires boost::mp11::mp_similar<typename urd::record_of<V>::Referenced, Argument<ConceptTag>>::value
    { return self().template get<&urd::record_of<V>::reference>(); }

    auto get_value() const noexcept
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
