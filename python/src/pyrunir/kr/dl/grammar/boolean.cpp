#include "../data_bindings.hpp"
#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/grammar/boolean_view.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl
{
namespace
{

template<typename Tag>
void bind_boolean_data(nb::module_& m, const char* name)
{
    using Data = urd::data<runir::kr::dl::grammar::Boolean<Tag>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    urd::python::add_comparison(cls);
    python::def_boolean_data_constructors<Tag,
                                          runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::BooleanTag>,
                                          runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>,
                                          runir::kr::dl::grammar::ConstructorRepository>(cls);

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def_rw("predicate", &Data::predicate).def_rw("polarity", &Data::polarity);
    else
        cls.def_rw("arg", &Data::arg);
}

template<typename Tag>
void bind_boolean_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::grammar::Boolean<Tag>;
    using View = urd::view<Type, runir::kr::dl::grammar::ConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::index);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def("get_predicate", &View::get_predicate, nb::keep_alive<0, 1>()).def("get_polarity", &View::get_polarity);
    else
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_grammar_boolean(nb::module_& m)
{
    using StaticState = runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>;
    using FluentState = runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>;
    using DerivedState = runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>;
    using StaticGoal = runir::kr::dl::grammar::Boolean<AtomicGoalTag<tyr::formalism::StaticTag>>;
    using FluentGoal = runir::kr::dl::grammar::Boolean<AtomicGoalTag<tyr::formalism::FluentTag>>;
    using DerivedGoal = runir::kr::dl::grammar::Boolean<AtomicGoalTag<tyr::formalism::DerivedTag>>;
    using Nonempty = runir::kr::dl::grammar::Boolean<NonemptyTag>;

    urd::python::bind_index<urd::index<StaticState>>(m, "BooleanAtomicStateStaticIndex");
    urd::python::bind_index<urd::index<FluentState>>(m, "BooleanAtomicStateFluentIndex");
    urd::python::bind_index<urd::index<DerivedState>>(m, "BooleanAtomicStateDerivedIndex");
    urd::python::bind_index<urd::index<StaticGoal>>(m, "BooleanAtomicGoalStaticIndex");
    urd::python::bind_index<urd::index<FluentGoal>>(m, "BooleanAtomicGoalFluentIndex");
    urd::python::bind_index<urd::index<DerivedGoal>>(m, "BooleanAtomicGoalDerivedIndex");
    urd::python::bind_index<urd::index<Nonempty>>(m, "BooleanNonemptyIndex");

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

}  // namespace runir::kr::dl
