#ifndef PYRUNIR_KR_DL_DATA_BINDINGS_HPP_
#define PYRUNIR_KR_DL_DATA_BINDINGS_HPP_

#include "module.hpp"

#include <concepts>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/declarations.hpp>
#include <type_traits>
#include <tyr/formalism/planning/declarations.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::python
{

// Value and view constructors of the description-logic constructor Data records. The semantic, grammar and CNF grammar
// records only differ in their operand symbols (Constructor, ConstructorOrNonTerminal, NonTerminal), which are passed as
// operand index types together with the repository type that the Python views use as context.

template<typename Tag, typename Class>
void def_predicate_data_constructors(Class& cls)
{
    using namespace nb::literals;
    cls.def(nb::init<ygg::Index<tyr::formalism::Predicate<typename Tag::FactKind>>, bool>(), "predicate"_a, "polarity"_a)
        .def(nb::init<tyr::formalism::planning::PredicateView<typename Tag::FactKind>, bool>(), "predicate"_a, "polarity"_a);
}

template<typename ArgIndex, typename Repository, typename Class>
void def_unary_data_constructors(Class& cls)
{
    using namespace nb::literals;
    cls.def(nb::init<ArgIndex>(), "arg"_a).def(nb::init<ygg::View<ArgIndex, Repository>>(), "arg"_a);
}

template<typename LhsIndex, typename RhsIndex, typename Repository, typename Class>
void def_binary_data_constructors(Class& cls)
{
    using namespace nb::literals;
    cls.def(nb::init<LhsIndex, RhsIndex>(), "lhs"_a, "rhs"_a)
        .def(nb::init<ygg::View<LhsIndex, Repository>, ygg::View<RhsIndex, Repository>>(), "lhs"_a, "rhs"_a);
}

template<typename ReferenceIndex, typename Repository, typename Class>
void def_reference_data_constructors(Class& cls)
{
    using namespace nb::literals;
    cls.def(nb::init<ReferenceIndex>(), "reference"_a).def(nb::init<ygg::View<ReferenceIndex, Repository>>(), "reference"_a);
}

template<typename Tag, typename ConceptIndex, typename RoleIndex, typename Repository, typename Class>
void def_concept_data_constructors(Class& cls)
{
    using namespace nb::literals;
    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        def_predicate_data_constructors<Tag>(cls);
    else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag>)
        def_binary_data_constructors<ConceptIndex, ConceptIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, ValueRestrictionTag> || std::same_as<Tag, ExistentialQuantificationTag>)
        def_binary_data_constructors<RoleIndex, ConceptIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, RoleValueMapTag> || std::same_as<Tag, AgreementTag>)
        def_binary_data_constructors<RoleIndex, RoleIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, NegationTag>)
        def_unary_data_constructors<ConceptIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag> || std::same_as<Tag, AtMostNumberRestrictionTag>
                       || std::same_as<Tag, ExactNumberRestrictionTag>)
        cls.def(nb::init<ygg::uint_t, RoleIndex>(), "n"_a, "role"_a).def(nb::init<ygg::uint_t, ygg::View<RoleIndex, Repository>>(), "n"_a, "role"_a);
    else if constexpr (std::same_as<Tag, QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, QualifiedAtMostNumberRestrictionTag>
                       || std::same_as<Tag, QualifiedExactNumberRestrictionTag>)
        cls.def(nb::init<ygg::uint_t, RoleIndex, ConceptIndex>(), "n"_a, "role"_a, "concept"_a)
            .def(nb::init<ygg::uint_t, ygg::View<RoleIndex, Repository>, ygg::View<ConceptIndex, Repository>>(), "n"_a, "role"_a, "concept"_a);
    else if constexpr (std::same_as<Tag, RoleFillersTag>)
        cls.def(nb::init<RoleIndex, ygg::IndexList<tyr::formalism::Object>>(), "role"_a, "objects"_a)
            .def(nb::init<ygg::View<RoleIndex, Repository>, const tyr::formalism::planning::ObjectViewList&>(), "role"_a, "objects"_a);
    else if constexpr (std::same_as<Tag, OneOfTag>)
        cls.def(nb::init<ygg::IndexList<tyr::formalism::Object>>(), "objects"_a).def(nb::init<const tyr::formalism::planning::ObjectViewList&>(), "objects"_a);
    else if constexpr (std::same_as<Tag, NominalTag>)
        cls.def(nb::init<ygg::Index<tyr::formalism::Object>>(), "object"_a).def(nb::init<tyr::formalism::planning::ObjectView>(), "object"_a);
    else if constexpr (std::same_as<Tag, RegisterTag>)
        def_reference_data_constructors<ygg::Index<Register<ConceptTag>>, Repository>(cls);
    else if constexpr (std::same_as<Tag, ArgumentTag<ConceptTag>>)
        def_reference_data_constructors<ygg::Index<Argument<ConceptTag>>, Repository>(cls);
    else
        static_assert(std::same_as<Tag, BotTag> || std::same_as<Tag, TopTag>);
}

template<typename Tag, typename ConceptIndex, typename RoleIndex, typename Repository, typename Class>
void def_role_data_constructors(Class& cls)
{
    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        def_predicate_data_constructors<Tag>(cls);
    else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag> || std::same_as<Tag, CompositionTag>)
        def_binary_data_constructors<RoleIndex, RoleIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, RestrictionTag>)
        def_binary_data_constructors<RoleIndex, ConceptIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, ComplementTag> || std::same_as<Tag, InverseTag> || std::same_as<Tag, TransitiveClosureTag>
                       || std::same_as<Tag, ReflexiveTransitiveClosureTag>)
        def_unary_data_constructors<RoleIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, IdentityTag>)
        def_unary_data_constructors<ConceptIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, RegisterTag>)
        def_reference_data_constructors<ygg::Index<Register<RoleTag>>, Repository>(cls);
    else if constexpr (std::same_as<Tag, ArgumentTag<RoleTag>>)
        def_reference_data_constructors<ygg::Index<Argument<RoleTag>>, Repository>(cls);
    else
        static_assert(std::same_as<Tag, UniversalTag>);
}

template<typename Tag, typename BooleanIndex, typename NumericalIndex, typename Repository, typename Class>
void def_boolean_data_constructors(Class& cls)
{
    using namespace nb::literals;
    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        def_predicate_data_constructors<Tag>(cls);
    else if constexpr (std::same_as<Tag, NonemptyTag>)
        cls.def(nb::init<typename Class::Type::Arg>(), "arg"_a).def(nb::init<typename Class::Type::template ViewVariant<Repository>>(), "arg"_a);
    else if constexpr (std::same_as<Tag, ArgumentTag<BooleanTag>>)
        def_reference_data_constructors<ygg::Index<Argument<BooleanTag>>, Repository>(cls);
    else if constexpr (ComparisonTag<Tag>)
    {
        using OperandIndex = std::conditional_t<std::same_as<comparison_operand_t<Tag>, BooleanTag>, BooleanIndex, NumericalIndex>;
        def_binary_data_constructors<OperandIndex, OperandIndex, Repository>(cls);
    }
    else if constexpr (LogicalBinaryTag<Tag>)
        def_binary_data_constructors<BooleanIndex, BooleanIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, NotTag>)
        def_unary_data_constructors<BooleanIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, BooleanConstantTag>)
        cls.def(nb::init<bool>(), "identifier"_a);
    else
        static_assert(ygg::dependent_false<Tag>::value);
}

template<typename Tag, typename ConceptIndex, typename RoleIndex, typename NumericalIndex, typename Repository, typename Class>
void def_numerical_data_constructors(Class& cls)
{
    using namespace nb::literals;
    if constexpr (std::same_as<Tag, CountTag>)
        cls.def(nb::init<typename Class::Type::Arg>(), "arg"_a).def(nb::init<typename Class::Type::template ViewVariant<Repository>>(), "arg"_a);
    else if constexpr (std::same_as<Tag, DistanceTag>)
        cls.def(nb::init<ConceptIndex, RoleIndex, ConceptIndex>(), "lhs"_a, "mid"_a, "rhs"_a)
            .def(nb::init<ygg::View<ConceptIndex, Repository>, ygg::View<RoleIndex, Repository>, ygg::View<ConceptIndex, Repository>>(),
                 "lhs"_a,
                 "mid"_a,
                 "rhs"_a);
    else if constexpr (std::same_as<Tag, NumericalConstantTag>)
        cls.def(nb::init<ygg::uint_t>(), "identifier"_a);
    else if constexpr (NumericalBinaryTag<Tag>)
        def_binary_data_constructors<NumericalIndex, NumericalIndex, Repository>(cls);
    else if constexpr (std::same_as<Tag, ArgumentTag<NumericalTag>>)
        def_reference_data_constructors<ygg::Index<Argument<NumericalTag>>, Repository>(cls);
    else
        static_assert(ygg::dependent_false<Tag>::value);
}

}  // namespace runir::kr::dl::python

#endif
