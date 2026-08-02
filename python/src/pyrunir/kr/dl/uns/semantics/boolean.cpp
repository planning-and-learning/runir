#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/boolean_view.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::uns
{
namespace
{

template<typename Tag>
void bind_boolean_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, Tag>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(cls);

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def_rw("predicate", &Data::predicate).def_rw("polarity", &Data::polarity);
    else if constexpr (std::same_as<Tag, NonemptyTag> || std::same_as<Tag, NotTag>)
        cls.def_rw("arg", &Data::arg);
    else if constexpr (ComparisonTag<Tag> || LogicalBinaryTag<Tag>)
        cls.def_rw("lhs", &Data::lhs).def_rw("rhs", &Data::rhs);
    else
        cls.def_rw("identifier", &Data::identifier);
}

template<typename Tag>
void bind_boolean_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, Tag>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::UnsConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    cls.def("syntactic_complexity", [](View view) { return runir::kr::dl::semantics::syntactic_complexity(view); });

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def("get_predicate", &View::get_predicate, nb::keep_alive<0, 1>()).def("get_polarity", &View::get_polarity);
    else if constexpr (std::same_as<Tag, NonemptyTag> || std::same_as<Tag, NotTag>)
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
    else if constexpr (ComparisonTag<Tag> || LogicalBinaryTag<Tag>)
        cls.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>()).def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
    else
        cls.def("get_value", &View::get_value);
}

}  // namespace

void bind_semantics_boolean(nb::module_& m)
{
    using StaticState = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>;
    using FluentState = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>;
    using DerivedState = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>;
    using StaticGoal = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>;
    using FluentGoal = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>;
    using DerivedGoal = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>;
    using Nonempty = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, NonemptyTag>;
    using BooleanEq = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, EqTag<BooleanTag>>;
    using BooleanNeq = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, NeqTag<BooleanTag>>;
    using BooleanLt = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, LtTag<BooleanTag>>;
    using BooleanLe = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, LeTag<BooleanTag>>;
    using BooleanGt = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, GtTag<BooleanTag>>;
    using BooleanGe = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, GeTag<BooleanTag>>;
    using NumericalEq = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, EqTag<NumericalTag>>;
    using NumericalNeq = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, NeqTag<NumericalTag>>;
    using NumericalLt = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, LtTag<NumericalTag>>;
    using NumericalLe = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, LeTag<NumericalTag>>;
    using NumericalGt = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, GtTag<NumericalTag>>;
    using NumericalGe = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, GeTag<NumericalTag>>;
    using Constant = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, BooleanConstantTag>;
    using And = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, AndTag>;
    using Or = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, OrTag>;
    using Not = runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, NotTag>;

    ygg::bind_index<ygg::Index<StaticState>>(m, "BooleanAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<FluentState>>(m, "BooleanAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<DerivedState>>(m, "BooleanAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<StaticGoal>>(m, "BooleanAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<FluentGoal>>(m, "BooleanAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<DerivedGoal>>(m, "BooleanAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<Nonempty>>(m, "BooleanNonemptyIndex");
    ygg::bind_index<ygg::Index<BooleanEq>>(m, "BooleanEqIndex");
    ygg::bind_index<ygg::Index<BooleanNeq>>(m, "BooleanNeqIndex");
    ygg::bind_index<ygg::Index<BooleanLt>>(m, "BooleanLtIndex");
    ygg::bind_index<ygg::Index<BooleanLe>>(m, "BooleanLeIndex");
    ygg::bind_index<ygg::Index<BooleanGt>>(m, "BooleanGtIndex");
    ygg::bind_index<ygg::Index<BooleanGe>>(m, "BooleanGeIndex");
    ygg::bind_index<ygg::Index<NumericalEq>>(m, "NumericalEqIndex");
    ygg::bind_index<ygg::Index<NumericalNeq>>(m, "NumericalNeqIndex");
    ygg::bind_index<ygg::Index<NumericalLt>>(m, "NumericalLtIndex");
    ygg::bind_index<ygg::Index<NumericalLe>>(m, "NumericalLeIndex");
    ygg::bind_index<ygg::Index<NumericalGt>>(m, "NumericalGtIndex");
    ygg::bind_index<ygg::Index<NumericalGe>>(m, "NumericalGeIndex");
    ygg::bind_index<ygg::Index<Constant>>(m, "BooleanConstantIndex");
    ygg::bind_index<ygg::Index<And>>(m, "BooleanAndIndex");
    ygg::bind_index<ygg::Index<Or>>(m, "BooleanOrIndex");
    ygg::bind_index<ygg::Index<Not>>(m, "BooleanNotIndex");

    bind_boolean_data<AtomicStateTag<tyr::formalism::StaticTag>>(m, "BooleanAtomicStateStaticData");
    bind_boolean_data<AtomicStateTag<tyr::formalism::FluentTag>>(m, "BooleanAtomicStateFluentData");
    bind_boolean_data<AtomicStateTag<tyr::formalism::DerivedTag>>(m, "BooleanAtomicStateDerivedData");
    bind_boolean_data<AtomicGoalTag<tyr::formalism::StaticTag>>(m, "BooleanAtomicGoalStaticData");
    bind_boolean_data<AtomicGoalTag<tyr::formalism::FluentTag>>(m, "BooleanAtomicGoalFluentData");
    bind_boolean_data<AtomicGoalTag<tyr::formalism::DerivedTag>>(m, "BooleanAtomicGoalDerivedData");
    bind_boolean_data<NonemptyTag>(m, "BooleanNonemptyData");
    bind_boolean_data<EqTag<BooleanTag>>(m, "BooleanEqData");
    bind_boolean_data<NeqTag<BooleanTag>>(m, "BooleanNeqData");
    bind_boolean_data<LtTag<BooleanTag>>(m, "BooleanLtData");
    bind_boolean_data<LeTag<BooleanTag>>(m, "BooleanLeData");
    bind_boolean_data<GtTag<BooleanTag>>(m, "BooleanGtData");
    bind_boolean_data<GeTag<BooleanTag>>(m, "BooleanGeData");
    bind_boolean_data<EqTag<NumericalTag>>(m, "NumericalEqData");
    bind_boolean_data<NeqTag<NumericalTag>>(m, "NumericalNeqData");
    bind_boolean_data<LtTag<NumericalTag>>(m, "NumericalLtData");
    bind_boolean_data<LeTag<NumericalTag>>(m, "NumericalLeData");
    bind_boolean_data<GtTag<NumericalTag>>(m, "NumericalGtData");
    bind_boolean_data<GeTag<NumericalTag>>(m, "NumericalGeData");
    bind_boolean_data<BooleanConstantTag>(m, "BooleanConstantData");
    bind_boolean_data<AndTag>(m, "BooleanAndData");
    bind_boolean_data<OrTag>(m, "BooleanOrData");
    bind_boolean_data<NotTag>(m, "BooleanNotData");

    bind_boolean_view<AtomicStateTag<tyr::formalism::StaticTag>>(m, "BooleanAtomicStateStatic");
    bind_boolean_view<AtomicStateTag<tyr::formalism::FluentTag>>(m, "BooleanAtomicStateFluent");
    bind_boolean_view<AtomicStateTag<tyr::formalism::DerivedTag>>(m, "BooleanAtomicStateDerived");
    bind_boolean_view<AtomicGoalTag<tyr::formalism::StaticTag>>(m, "BooleanAtomicGoalStatic");
    bind_boolean_view<AtomicGoalTag<tyr::formalism::FluentTag>>(m, "BooleanAtomicGoalFluent");
    bind_boolean_view<AtomicGoalTag<tyr::formalism::DerivedTag>>(m, "BooleanAtomicGoalDerived");
    bind_boolean_view<NonemptyTag>(m, "BooleanNonempty");
    bind_boolean_view<EqTag<BooleanTag>>(m, "BooleanEq");
    bind_boolean_view<NeqTag<BooleanTag>>(m, "BooleanNeq");
    bind_boolean_view<LtTag<BooleanTag>>(m, "BooleanLt");
    bind_boolean_view<LeTag<BooleanTag>>(m, "BooleanLe");
    bind_boolean_view<GtTag<BooleanTag>>(m, "BooleanGt");
    bind_boolean_view<GeTag<BooleanTag>>(m, "BooleanGe");
    bind_boolean_view<EqTag<NumericalTag>>(m, "NumericalEq");
    bind_boolean_view<NeqTag<NumericalTag>>(m, "NumericalNeq");
    bind_boolean_view<LtTag<NumericalTag>>(m, "NumericalLt");
    bind_boolean_view<LeTag<NumericalTag>>(m, "NumericalLe");
    bind_boolean_view<GtTag<NumericalTag>>(m, "NumericalGt");
    bind_boolean_view<GeTag<NumericalTag>>(m, "NumericalGe");
    bind_boolean_view<BooleanConstantTag>(m, "BooleanConstant");
    bind_boolean_view<AndTag>(m, "BooleanAnd");
    bind_boolean_view<OrTag>(m, "BooleanOr");
    bind_boolean_view<NotTag>(m, "BooleanNot");
}

}  // namespace runir::kr::dl::uns
