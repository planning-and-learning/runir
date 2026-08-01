#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/boolean_view.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::base
{
namespace
{

template<typename Tag>
void bind_boolean_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, Tag>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(cls);

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def_rw("predicate", &Data::predicate).def_rw("polarity", &Data::polarity);
    else
        cls.def_rw("arg", &Data::arg);
}

template<typename Tag>
void bind_boolean_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, Tag>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::BaseConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    cls.def("syntactic_complexity", [](View view) { return runir::kr::dl::semantics::syntactic_complexity(view); });

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def("get_predicate", &View::get_predicate, nb::keep_alive<0, 1>()).def("get_polarity", &View::get_polarity);
    else
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_semantics_boolean(nb::module_& m)
{
    using StaticState = runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>;
    using FluentState = runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>;
    using DerivedState = runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>;
    using StaticGoal = runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>;
    using FluentGoal = runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>;
    using DerivedGoal = runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>;
    using Nonempty = runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, NonemptyTag>;

    ygg::bind_index<ygg::Index<StaticState>>(m, "BooleanAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<FluentState>>(m, "BooleanAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<DerivedState>>(m, "BooleanAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<StaticGoal>>(m, "BooleanAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<FluentGoal>>(m, "BooleanAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<DerivedGoal>>(m, "BooleanAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<Nonempty>>(m, "BooleanNonemptyIndex");

    bind_boolean_data<AtomicStateTag<tyr::formalism::StaticTag>>(m, "BooleanAtomicStateStaticData");
    bind_boolean_data<AtomicStateTag<tyr::formalism::FluentTag>>(m, "BooleanAtomicStateFluentData");
    bind_boolean_data<AtomicStateTag<tyr::formalism::DerivedTag>>(m, "BooleanAtomicStateDerivedData");
    bind_boolean_data<AtomicGoalTag<tyr::formalism::StaticTag>>(m, "BooleanAtomicGoalStaticData");
    bind_boolean_data<AtomicGoalTag<tyr::formalism::FluentTag>>(m, "BooleanAtomicGoalFluentData");
    bind_boolean_data<AtomicGoalTag<tyr::formalism::DerivedTag>>(m, "BooleanAtomicGoalDerivedData");
    bind_boolean_data<NonemptyTag>(m, "BooleanNonemptyData");

    bind_boolean_view<AtomicStateTag<tyr::formalism::StaticTag>>(m, "BooleanAtomicStateStatic");
    bind_boolean_view<AtomicStateTag<tyr::formalism::FluentTag>>(m, "BooleanAtomicStateFluent");
    bind_boolean_view<AtomicStateTag<tyr::formalism::DerivedTag>>(m, "BooleanAtomicStateDerived");
    bind_boolean_view<AtomicGoalTag<tyr::formalism::StaticTag>>(m, "BooleanAtomicGoalStatic");
    bind_boolean_view<AtomicGoalTag<tyr::formalism::FluentTag>>(m, "BooleanAtomicGoalFluent");
    bind_boolean_view<AtomicGoalTag<tyr::formalism::DerivedTag>>(m, "BooleanAtomicGoalDerived");
    bind_boolean_view<NonemptyTag>(m, "BooleanNonempty");
}

}  // namespace runir::kr::dl::base
