#include "runir/kr/ps/ext/dl/parser.hpp"

#include "kr/parser/resolution.hpp"
#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/grammar/parser/parser.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/errors.hpp"
#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/ps/ext/canonicalization.hpp"
#include "runir/kr/ps/ext/dl/parser/parser.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <yggdrasil/containers/variant.hpp>

namespace runir::kr::ps::ext::dl
{
namespace
{
template<typename Entity>
struct ReferenceTable
{
    std::unordered_map<std::string, ygg::Index<Entity>> by_name;
    std::vector<ygg::Index<Entity>> by_identifier;
};

struct ModuleReferences
{
    ReferenceTable<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>> concept_arguments;
    ReferenceTable<runir::kr::dl::Argument<runir::kr::dl::RoleTag>> role_arguments;
    ReferenceTable<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>> boolean_arguments;
    ReferenceTable<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>> numerical_arguments;
    ReferenceTable<runir::kr::dl::Register<runir::kr::dl::ConceptTag>> concept_registers;
    ReferenceTable<runir::kr::dl::Register<runir::kr::dl::RoleTag>> role_registers;
};

struct ConstructorContext
{
    runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository;
    runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder;
    const runir::kr::parser::DiagnosticContext& diagnostics;
    const ModuleReferences* references;
};

struct Builders
{
    runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& dl;
    runir::kr::ps::ext::Builder& ps;
};

template<typename T>
struct IsForwardAst : std::false_type
{
};

template<typename T>
struct IsForwardAst<boost::spirit::x3::forward_ast<T>> : std::true_type
{
};

template<typename T>
decltype(auto) unwrap(const T& value) noexcept
{
    if constexpr (IsForwardAst<T>::value)
        return value.get();
    else
        return (value);
}

template<typename T>
auto intern(Repository& repository, ygg::Data<T>& data)
{
    return runir::kr::ps::ext::get_or_create(repository, data).first;
}

template<typename T>
auto intern(const ConstructorContext& context, ygg::Data<T>& data)
{
    return runir::kr::dl::get_or_create(context.repository, data).first;
}

template<runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(const ConstructorContext& context, ygg::Index<T> index)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Constructor<runir::kr::ExtFamilyTag, Category>>(context.builder);
    data->variant = index;
    return intern(context, *data);
}

auto parse_concept(const std::string& description,
                   tyr::formalism::planning::DomainView domain,
                   runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                   runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder,
                   runir::kr::parser::DiagnosticContext& diagnostics)
    -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>;

auto parse_role(const std::string& description,
                tyr::formalism::planning::DomainView domain,
                runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder,
                runir::kr::parser::DiagnosticContext& diagnostics) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>;

auto parse_boolean(const std::string& description,
                   tyr::formalism::planning::DomainView domain,
                   runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                   runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder,
                   runir::kr::parser::DiagnosticContext& diagnostics)
    -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>;

auto parse_numerical(const std::string& description,
                     tyr::formalism::planning::DomainView domain,
                     runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                     runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder,
                     runir::kr::parser::DiagnosticContext& diagnostics)
    -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>;

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::ExtFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       const ConstructorContext& context) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor_or_non_terminal(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<runir::kr::ExtFamilyTag, Category>& node,
                                       tyr::formalism::planning::DomainView domain,
                                       const ConstructorContext& context) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, runir::kr::dl::grammar::ast::NonTerminal<runir::kr::ExtFamilyTag, Category>>)
                context.diagnostics.throw_at(unwrapped.name,
                                             runir::kr::InvalidExpressionError("Ext module DL expressions cannot reference "
                                                                               "grammar nonterminals."));
            else
                return parse_constructor(unwrapped, domain, context);
        },
        node.get());
}

template<typename Entity, typename Identifier, typename Reference>
ygg::Index<Entity>
resolve_reference(const Reference& reference, const ReferenceTable<Entity>* declarations, const char* kind, const ConstructorContext& context)
{
    return boost::apply_visitor(
        [&](const auto& concrete) -> ygg::Index<Entity>
        {
            using Concrete = std::remove_cvref_t<decltype(concrete)>;
            if constexpr (std::same_as<Concrete, runir::kr::dl::grammar::ast::NumericReference>)
            {
                if (declarations)
                {
                    if (concrete.value >= declarations->by_identifier.size())
                        context.diagnostics.throw_at(concrete, runir::kr::UndefinedSymbolError(kind, std::to_string(concrete.value)));
                    return declarations->by_identifier[concrete.value];
                }

                try
                {
                    auto data = runir::kr::dl::checkout<Entity>(context.builder);
                    data->name = std::to_string(concrete.value);
                    data->identifier = Identifier(concrete.value);
                    return intern(context, *data).get_index();
                }
                catch (const std::out_of_range&)
                {
                    context.diagnostics.throw_at(concrete, runir::kr::UndefinedSymbolError(kind, std::to_string(concrete.value)));
                }
            }
            else
            {
                if (declarations)
                {
                    const auto it = declarations->by_name.find(concrete.text);
                    if (it != declarations->by_name.end())
                        return it->second;
                    context.diagnostics.throw_at(concrete, runir::kr::UndefinedSymbolError(kind, concrete.text));
                }
                context.diagnostics.throw_at(concrete, runir::kr::InvalidExpressionError("Expected numeric DL reference: " + concrete.text));
            }
        },
        reference.get());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptBot<runir::kr::ExtFamilyTag>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::BotTag>>(context.builder);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptTop<runir::kr::ExtFamilyTag>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::TopTag>>(context.builder);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        1,
        "ConceptAtomicState",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = true;
            return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        1,
        "ConceptAtomicGoal",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = node.polarity;
            return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
        });
}

template<typename Tag, typename Ast>
auto parse_binary_concept(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, Tag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptUnion<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNegation<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::NegationTag>>(context.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<typename Tag, typename Ast>
auto parse_number_restriction(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, Tag>>(context.builder);
    data->n = node.n;
    data->role = role.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_number_restriction<runir::kr::dl::AtLeastNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_number_restriction<runir::kr::dl::AtMostNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_number_restriction<runir::kr::dl::ExactNumberRestrictionTag>(node, domain, context);
}

template<typename Tag, typename Ast>
auto parse_qualified_number_restriction(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context);
    const auto concept_view = parse_constructor_or_non_terminal(node.concept_, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, Tag>>(context.builder);
    data->n = node.n;
    data->role = role.get_index();
    data->concept_ = concept_view.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_qualified_number_restriction<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_qualified_number_restriction<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_qualified_number_restriction<runir::kr::dl::QualifiedExactNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMap<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::RoleValueMapTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAgreement<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::AgreementTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleFillers<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RoleFillersTag>>(context.builder);
    data->role = role.get_index();
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptOneOf<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::OneOfTag>>(context.builder);
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNominal<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::NominalTag>>(context.builder);
    data->object = runir::kr::parser::require_object(domain, node.object_name, context.diagnostics);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRegister<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Register<runir::kr::dl::ConceptTag>, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>(
        node.reference,
        context.references ? &context.references->concept_registers : nullptr,
        "concept register",
        context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptArgument<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           const ConstructorContext& context)
{
    auto data =
        runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>(
        node.reference,
        context.references ? &context.references->concept_arguments : nullptr,
        "concept argument",
        context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleUniversal<runir::kr::ExtFamilyTag>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::UniversalTag>>(context.builder);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        2,
        "RoleAtomicState",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = true;
            return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        2,
        "RoleAtomicGoal",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = node.polarity;
            return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
        });
}

template<typename Tag, typename Ast>
auto parse_binary_role(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, Tag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIntersection<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleUnion<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleComposition<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, context);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, Tag>>(context.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleComplement<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleInverse<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleRestriction<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RestrictionTag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIdentity<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::IdentityTag>>(context.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleRegister<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Register<runir::kr::dl::RoleTag>, runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag>>(
        node.reference,
        context.references ? &context.references->role_registers : nullptr,
        "role register",
        context);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleArgument<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::RoleTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::RoleTag>>(
        node.reference,
        context.references ? &context.references->role_arguments : nullptr,
        "role argument",
        context);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        0,
        "BooleanAtomicState",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = node.polarity;
            return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicGoal<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        0,
        "BooleanAtomicGoal",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = node.polarity;
            return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    using Data = ygg::Data<runir::kr::dl::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NonemptyTag>>;
    const auto arg = boost::apply_visitor([&](const auto& value) -> typename Data::ConstructorVariant
                                          { return parse_constructor_or_non_terminal(unwrap(value), domain, context).get_index(); },
                                          node.arg.get());
    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NonemptyTag>>(context.builder);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanArgument<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           const ConstructorContext& context)
{
    auto data =
        runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>(
        node.reference,
        context.references ? &context.references->boolean_arguments : nullptr,
        "boolean argument",
        context);
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalCount<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    using Data = ygg::Data<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::CountTag>>;
    const auto arg = boost::apply_visitor([&](const auto& value) -> typename Data::ConstructorVariant
                                          { return parse_constructor_or_non_terminal(unwrap(value), domain, context).get_index(); },
                                          node.arg.get());
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::CountTag>>(context.builder);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDistance<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto mid = parse_constructor_or_non_terminal(node.mid, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::DistanceTag>>(context.builder);
    data->lhs = lhs.get_index();
    data->mid = mid.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalArgument<runir::kr::ExtFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           const ConstructorContext& context)
{
    auto data =
        runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>(
        node.reference,
        context.references ? &context.references->numerical_arguments : nullptr,
        "numerical argument",
        context);
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::ExtFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       const ConstructorContext& context) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, context); }, node.get());
}

template<typename T>
struct AstCategory
{
};

template<runir::kr::dl::CategoryTag Category>
struct AstCategory<ast::Argument<Category>>
{
    using Type = Category;
};

template<runir::kr::dl::CategoryTag Category>
struct AstCategory<ast::Register<Category>>
{
    using Type = Category;
};

template<runir::kr::dl::CategoryTag Category>
struct AstCategory<ast::Feature<Category>>
{
    using Type = Category;
};

template<runir::kr::dl::CategoryTag Category>
struct AstCategory<ast::LoadRule<Category>>
{
    using Type = Category;
};

template<runir::kr::dl::CategoryTag Category>
auto intern_argument(Repository& repository,
                     runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder,
                     const ast::Argument<Category>& argument,
                     ygg::uint_t identifier)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Argument<Category>>(builder);
    data->name = argument.symbol.text;
    data->identifier = runir::kr::dl::ArgumentIdentifier<Category>(identifier);
    return runir::kr::dl::get_or_create(repository.get_dl_repository(), *data).first;
}

struct SignatureCounts
{
    std::size_t concepts = 0;
    std::size_t roles = 0;
    std::size_t booleans = 0;
    std::size_t numericals = 0;

    bool operator==(const SignatureCounts&) const noexcept = default;
};

std::string signature_text(const SignatureCounts& counts)
{
    return "concept=" + std::to_string(counts.concepts) + ", role=" + std::to_string(counts.roles) + ", boolean=" + std::to_string(counts.booleans)
           + ", numerical=" + std::to_string(counts.numericals);
}

template<runir::kr::dl::CategoryTag Category>
void increment(SignatureCounts& counts)
{
    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
        ++counts.concepts;
    else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
        ++counts.roles;
    else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
        ++counts.booleans;
    else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
        ++counts.numericals;
}

SignatureCounts signature_counts(const ast::Module& module)
{
    auto result = SignatureCounts {};
    for (const auto& argument : module.arguments)
        boost::apply_visitor([&](const auto& concrete) { increment<typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type>(result); },
                             argument.get());
    return result;
}

template<runir::kr::dl::CategoryTag Category>
void validate_unique_names(const std::vector<ast::Register<Category>>& declarations, const char* kind, const runir::kr::parser::DiagnosticContext& diagnostics)
{
    if (declarations.size() > runir::kr::dl::num_registers)
        diagnostics.throw_at(declarations[runir::kr::dl::num_registers],
                             runir::kr::InvalidExpressionError(std::string(kind) + " declares more registers than the supported maximum of "
                                                               + std::to_string(runir::kr::dl::num_registers)));

    auto names = std::unordered_set<std::string> {};
    for (const auto& declaration : declarations)
        if (!names.emplace(declaration.symbol.text).second)
            diagnostics.throw_at(declaration.symbol, runir::kr::DuplicateDefinitionError(std::string(kind) + " register", declaration.symbol.text));
}

template<runir::kr::dl::CategoryTag Category>
std::vector<ast::Register<Category>> collect_registers(const std::vector<ast::RegisterVariant>& registers)
{
    auto result = std::vector<ast::Register<Category>> {};
    for (const auto& reg : registers)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using ConcreteCategory = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<ConcreteCategory, Category>)
                    result.push_back(concrete);
            },
            reg.get());
    }
    return result;
}

void validate_module_declarations(const ast::Module& module, const runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto concept_arguments = std::unordered_set<std::string> {};
    auto role_arguments = std::unordered_set<std::string> {};
    auto boolean_arguments = std::unordered_set<std::string> {};
    auto numerical_arguments = std::unordered_set<std::string> {};

    const auto add_argument = [&](auto& names, const ast::Identifier& symbol, const char* kind)
    {
        if (!names.emplace(symbol.text).second)
            diagnostics.throw_at(symbol, runir::kr::DuplicateDefinitionError(std::string(kind) + " argument", symbol.text));
    };

    for (const auto& argument : module.arguments)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                    add_argument(concept_arguments, concrete.symbol, "concept");
                else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
                    add_argument(role_arguments, concrete.symbol, "role");
                else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
                    add_argument(boolean_arguments, concrete.symbol, "boolean");
                else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
                    add_argument(numerical_arguments, concrete.symbol, "numerical");
            },
            argument.get());
    }

    validate_unique_names(collect_registers<runir::kr::dl::ConceptTag>(module.registers), "concept", diagnostics);
    validate_unique_names(collect_registers<runir::kr::dl::RoleTag>(module.registers), "role", diagnostics);

    auto feature_symbols = std::unordered_set<std::string> {};
    for (const auto& feature : module.features)
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                if (!feature_symbols.emplace(concrete.symbol.text).second)
                    diagnostics.throw_at(concrete.symbol, runir::kr::DuplicateDefinitionError("feature", concrete.symbol.text));
            },
            feature.get());

    auto rule_symbols = std::unordered_set<std::string> {};
    for (const auto& entry : module.rule_entries)
        if (!rule_symbols.emplace(entry.symbol.text).second)
            diagnostics.throw_at(entry.symbol, runir::kr::DuplicateDefinitionError("rule", entry.symbol.text));
}

void append_argument(Repository& repository,
                     runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder,
                     ygg::Data<Module>& data,
                     ModuleReferences& references,
                     const ast::ArgumentVariant& argument,
                     ygg::uint_t& concept_identifier,
                     ygg::uint_t& role_identifier,
                     ygg::uint_t& boolean_identifier,
                     ygg::uint_t& numerical_identifier)
{
    boost::apply_visitor(
        [&](const auto& concrete)
        {
            using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
            if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            {
                const auto index = intern_argument<Category>(repository, builder, concrete, concept_identifier++).get_index();
                data.concept_arguments.push_back(index);
                references.concept_arguments.by_name.emplace(concrete.symbol.text, index);
                references.concept_arguments.by_identifier.push_back(index);
            }
            else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            {
                const auto index = intern_argument<Category>(repository, builder, concrete, role_identifier++).get_index();
                data.role_arguments.push_back(index);
                references.role_arguments.by_name.emplace(concrete.symbol.text, index);
                references.role_arguments.by_identifier.push_back(index);
            }
            else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            {
                const auto index = intern_argument<Category>(repository, builder, concrete, boolean_identifier++).get_index();
                data.boolean_arguments.push_back(index);
                references.boolean_arguments.by_name.emplace(concrete.symbol.text, index);
                references.boolean_arguments.by_identifier.push_back(index);
            }
            else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            {
                const auto index = intern_argument<Category>(repository, builder, concrete, numerical_identifier++).get_index();
                data.numerical_arguments.push_back(index);
                references.numerical_arguments.by_name.emplace(concrete.symbol.text, index);
                references.numerical_arguments.by_identifier.push_back(index);
            }
        },
        argument.get());
}

template<typename FeatureTag, typename ConcreteFeatureTag>
auto intern_dl_feature(Repository& repository,
                       runir::kr::ps::ext::Builder& builder,
                       ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, ConcreteFeatureTag>> constructor,
                       const std::string& symbol)
{
    auto concrete_data = runir::kr::ps::ext::checkout<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>(builder);
    concrete_data->feature = constructor;
    concrete_data->symbol = symbol;
    const auto concrete = intern(repository, *concrete_data);
    auto feature_data = runir::kr::ps::ext::checkout<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>(builder);
    feature_data->variant = concrete.get_index();
    return intern(repository, *feature_data);
}

template<runir::kr::dl::CategoryTag Category>
void append_feature(
    Repository& repository,
    Builders& builders,
    ygg::Data<Module>& module_data,
    const ast::Feature<Category>& feature,
    tyr::formalism::planning::DomainView domain,
    std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>& concept_features,
    std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>>& role_features,
    std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>& numerical_features,
    const ModuleReferences& references,
    runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto context = ConstructorContext { repository.get_dl_repository(), builders.dl, diagnostics, &references };
    const auto constructor = parse_constructor(feature.expression, domain, context);
    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        const auto view = intern_dl_feature<runir::kr::dl::ConceptTag>(repository, builders.ps, constructor.get_index(), feature.symbol.text);
        concept_features.emplace(feature.symbol.text, view.get_index());
        module_data.concept_features.push_back(view.get_index());
    }
    else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        const auto view = intern_dl_feature<runir::kr::dl::RoleTag>(repository, builders.ps, constructor.get_index(), feature.symbol.text);
        role_features.emplace(feature.symbol.text, view.get_index());
        module_data.role_features.push_back(view.get_index());
    }
    else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
    {
        const auto view = intern_dl_feature<runir::kr::ps::dl::BooleanFeature>(repository, builders.ps, constructor.get_index(), feature.symbol.text);
        boolean_features.emplace(feature.symbol.text, view.get_index());
        module_data.boolean_features.push_back(view.get_index());
    }
    else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
    {
        const auto view = intern_dl_feature<runir::kr::ps::dl::NumericalFeature>(repository, builders.ps, constructor.get_index(), feature.symbol.text);
        numerical_features.emplace(feature.symbol.text, view.get_index());
        module_data.numerical_features.push_back(view.get_index());
    }
}

template<typename FeatureTag>
auto require_feature(const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>>& features,
                     const ast::Identifier& name,
                     const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto it = features.find(name.text);
    if (it == features.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("feature", name.text));
    return it->second;
}

template<typename FeatureTag, typename ObservationTag>
auto make_condition(Repository& repository,
                    runir::kr::ps::ext::Builder& builder,
                    ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>> feature)
{
    auto concrete_data =
        runir::kr::ps::ext::checkout<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>(builder);
    concrete_data->feature = feature;
    const auto concrete = intern(repository, *concrete_data);
    auto concrete_variant_data = runir::kr::ps::ext::checkout<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>(builder);
    concrete_variant_data->variant = concrete.get_index();
    const auto concrete_variant = intern(repository, *concrete_variant_data);
    auto variant_data = runir::kr::ps::ext::checkout<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>(builder);
    variant_data->variant = concrete_variant.get_index();
    return intern(repository, *variant_data);
}

template<typename FeatureTag, typename ObservationTag>
auto make_effect(Repository& repository, runir::kr::ps::ext::Builder& builder, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>> feature)
{
    auto concrete_data =
        runir::kr::ps::ext::checkout<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>(builder);
    concrete_data->feature = feature;
    const auto concrete = intern(repository, *concrete_data);
    auto concrete_variant_data = runir::kr::ps::ext::checkout<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>(builder);
    concrete_variant_data->variant = concrete.get_index();
    const auto concrete_variant = intern(repository, *concrete_variant_data);
    auto variant_data = runir::kr::ps::ext::checkout<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>(builder);
    variant_data->variant = concrete_variant.get_index();
    return intern(repository, *variant_data);
}

auto parse_condition(
    Repository& repository,
    runir::kr::ps::ext::Builder& builder,
    const ast::Condition& condition,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>& numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& observation)
        {
            using Observation = std::remove_cvref_t<decltype(observation)>;
            if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Positive>)
            {
                return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(
                    repository,
                    builder,
                    require_feature(boolean_features, condition.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Negative>)
            {
                return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(
                    repository,
                    builder,
                    require_feature(boolean_features, condition.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::EqualZero>)
            {
                return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>(
                    repository,
                    builder,
                    require_feature(numerical_features, condition.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::GreaterZero>)
            {
                return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>(
                    repository,
                    builder,
                    require_feature(numerical_features, condition.feature, diagnostics));
            }
        },
        condition.observation.get());
}

auto parse_effect(
    Repository& repository,
    runir::kr::ps::ext::Builder& builder,
    const ast::Effect& effect,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>& numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& observation)
        {
            using Observation = std::remove_cvref_t<decltype(observation)>;
            if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Positive>)
            {
                return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(
                    repository,
                    builder,
                    require_feature(boolean_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Negative>)
            {
                return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(
                    repository,
                    builder,
                    require_feature(boolean_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Unchanged>)
            {
                if (boolean_features.contains(effect.feature.text))
                    return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>(
                        repository,
                        builder,
                        require_feature(boolean_features, effect.feature, diagnostics));
                return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>(
                    repository,
                    builder,
                    require_feature(numerical_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Increases>)
            {
                return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>(
                    repository,
                    builder,
                    require_feature(numerical_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Decreases>)
            {
                return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>(
                    repository,
                    builder,
                    require_feature(numerical_features, effect.feature, diagnostics));
            }
        },
        effect.observation.get());
}

void append_conditions(
    Repository& repository,
    runir::kr::ps::ext::Builder& builder,
    const std::vector<ast::Condition>& observations,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>& numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics,
    ygg::IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>& result)
{
    result.reserve(result.size() + observations.size());
    for (const auto& observation : observations)
        result.push_back(parse_condition(repository, builder, observation, boolean_features, numerical_features, diagnostics).get_index());
}

void append_effects(
    Repository& repository,
    runir::kr::ps::ext::Builder& builder,
    const std::vector<ast::Effect>& observations,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>& numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics,
    ygg::IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>& result)
{
    result.reserve(result.size() + observations.size());
    for (const auto& observation : observations)
        result.push_back(parse_effect(repository, builder, observation, boolean_features, numerical_features, diagnostics).get_index());
}

auto parse_do_argument(
    const ast::SymbolExpression& expression,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>& concept_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return require_feature(concept_features, expression.symbol, diagnostics);
}

auto parse_call_argument(
    const ast::SymbolExpression& expression,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>& concept_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>>& role_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>& numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics) -> CallArgument
{
    const auto& name = expression.symbol.text;
    auto matches = 0;
    auto result = CallArgument {};
    if (const auto it = concept_features.find(name); it != concept_features.end())
    {
        result = it->second;
        ++matches;
    }
    if (const auto it = role_features.find(name); it != role_features.end())
    {
        result = it->second;
        ++matches;
    }
    if (const auto it = boolean_features.find(name); it != boolean_features.end())
    {
        result = it->second;
        ++matches;
    }
    if (const auto it = numerical_features.find(name); it != numerical_features.end())
    {
        result = it->second;
        ++matches;
    }

    if (matches == 0)
        diagnostics.throw_at(expression.symbol, runir::kr::UndefinedSymbolError("call-rule argument feature", name));
    if (matches > 1)
        diagnostics.throw_at(expression.symbol, runir::kr::InvalidExpressionError("Ambiguous call-rule argument feature: " + name));
    return result;
}

struct FeatureSymbolSets
{
    std::unordered_set<std::string> concepts;
    std::unordered_set<std::string> roles;
    std::unordered_set<std::string> booleans;
    std::unordered_set<std::string> numericals;
};

FeatureSymbolSets feature_symbols(const ast::Module& module)
{
    auto result = FeatureSymbolSets {};
    for (const auto& feature : module.features)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                    result.concepts.emplace(concrete.symbol.text);
                else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
                    result.roles.emplace(concrete.symbol.text);
                else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
                    result.booleans.emplace(concrete.symbol.text);
                else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
                    result.numericals.emplace(concrete.symbol.text);
            },
            feature.get());
    }
    return result;
}

void increment_signature_count(SignatureCounts& counts,
                               const FeatureSymbolSets& features,
                               const ast::Identifier& symbol,
                               const runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto matches = 0;
    if (features.concepts.contains(symbol.text))
    {
        ++counts.concepts;
        ++matches;
    }
    if (features.roles.contains(symbol.text))
    {
        ++counts.roles;
        ++matches;
    }
    if (features.booleans.contains(symbol.text))
    {
        ++counts.booleans;
        ++matches;
    }
    if (features.numericals.contains(symbol.text))
    {
        ++counts.numericals;
        ++matches;
    }
    if (matches == 0)
        diagnostics.throw_at(symbol, runir::kr::UndefinedSymbolError("call-rule argument feature", symbol.text));
    if (matches > 1)
        diagnostics.throw_at(symbol, runir::kr::InvalidExpressionError("Ambiguous call-rule argument feature: " + symbol.text));
}

SignatureCounts
call_argument_signature_counts(const ast::CallRule& rule, const FeatureSymbolSets& features, const runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto result = SignatureCounts {};
    for (const auto& argument : rule.arguments)
        increment_signature_count(result, features, argument.symbol, diagnostics);
    return result;
}

template<RuleKind Kind>
auto intern_rule_variant(Repository& repository, runir::kr::ps::ext::Builder& builder, ygg::Data<Rule<Kind>>& data, const std::string& symbol)
{
    const auto rule = intern(repository, data);
    auto variant_data = runir::kr::ps::ext::checkout<ps::Rule<ExtFamilyTag>>(builder);
    variant_data->symbol = symbol;
    variant_data->variant = rule.get_index();
    return intern(repository, *variant_data);
}

auto require_memory_state(const std::unordered_map<std::string, ygg::Index<MemoryState>>& memory_states,
                          const ast::Identifier& name,
                          const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto it = memory_states.find(name.text);
    if (it == memory_states.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("memory state", name.text));
    return it->second;
}

template<typename RegisterMap>
auto require_register(const RegisterMap& registers, const ast::Identifier& name, const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto it = registers.find(name.text);
    if (it == registers.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("register", name.text));
    return it->second;
}

auto find_module(const std::unordered_map<std::string, ygg::Index<ModuleSymbol>>& modules, const std::string& name)
{
    const auto it = modules.find(name);
    return it == modules.end() ? std::optional<ygg::Index<ModuleSymbol>> {} : std::optional(it->second);
}

auto find_action_arity(tyr::formalism::planning::DomainView domain, const std::string& name)
{
    for (const auto action : domain.get_actions())
        if (action.get_name() == name)
            return std::optional<ygg::uint_t>(action.get_original_arity());
    return std::optional<ygg::uint_t> {};
}

void validate_do_action(tyr::formalism::planning::DomainView domain, const ast::DoRule& rule, const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto arity = find_action_arity(domain, rule.action.text);
    if (!arity)
        diagnostics.throw_at(rule.action, runir::kr::UndefinedSymbolError("action", rule.action.text));

    if (rule.arguments.size() != *arity)
        diagnostics.throw_at(rule.action, runir::kr::ArityMismatchError("action " + rule.action.text, *arity, rule.arguments.size()));
}

template<runir::kr::dl::CategoryTag Category>
auto parse_load_rule(
    Repository& repository,
    runir::kr::ps::ext::Builder& builder,
    const ast::LoadRule<Category>& rule,
    ygg::Index<MemoryState> source,
    ygg::Index<MemoryState> target,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>& concept_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>>& role_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>& numerical_features,
    const ModuleReferences& references,
    const std::string& symbol,
    runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto data = runir::kr::ps::ext::checkout<Rule<LoadTag<Category>>>(builder);
    data->source = source;
    data->target = target;
    append_conditions(repository, builder, rule.conditions, boolean_features, numerical_features, diagnostics, data->conditions);
    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        data->feature = require_feature(concept_features, rule.feature, diagnostics);
        data->reg = require_register(references.concept_registers.by_name, rule.reg, diagnostics);
    }
    else
    {
        data->feature = require_feature(role_features, rule.feature, diagnostics);
        data->reg = require_register(references.role_registers.by_name, rule.reg, diagnostics);
    }
    return intern_rule_variant(repository, builder, *data, symbol);
}

auto parse_rule(
    Repository& repository,
    runir::kr::ps::ext::Builder& builder,
    const ast::Rule& rule,
    ygg::Index<MemoryState> source,
    ygg::Index<MemoryState> target,
    tyr::formalism::planning::DomainView domain,
    const std::unordered_map<std::string, ygg::Index<ModuleSymbol>>& modules,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>& concept_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>>& role_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>& numerical_features,
    const ModuleReferences& references,
    const std::string& symbol,
    runir::kr::parser::DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& concrete) -> RuleVariantView
        {
            using RuleAst = std::remove_cvref_t<decltype(concrete)>;
            if constexpr (requires { typename AstCategory<RuleAst>::Type; })
            {
                using Category = typename AstCategory<RuleAst>::Type;
                return parse_load_rule<Category>(repository,
                                                 builder,
                                                 concrete,
                                                 source,
                                                 target,
                                                 concept_features,
                                                 role_features,
                                                 boolean_features,
                                                 numerical_features,
                                                 references,
                                                 symbol,
                                                 diagnostics);
            }
            else if constexpr (std::same_as<RuleAst, ast::SketchRule>)
            {
                auto data = runir::kr::ps::ext::checkout<Rule<SketchTag>>(builder);
                data->source = source;
                data->target = target;
                append_conditions(repository, builder, concrete.conditions, boolean_features, numerical_features, diagnostics, data->conditions);
                append_effects(repository, builder, concrete.effects, boolean_features, numerical_features, diagnostics, data->effects);
                return intern_rule_variant(repository, builder, *data, symbol);
            }
            else if constexpr (std::same_as<RuleAst, ast::DoRule>)
            {
                validate_do_action(domain, concrete, diagnostics);
                auto data = runir::kr::ps::ext::checkout<Rule<DoTag>>(builder);
                data->source = source;
                data->target = target;
                data->action_name = concrete.action.text;
                append_conditions(repository, builder, concrete.conditions, boolean_features, numerical_features, diagnostics, data->conditions);
                append_effects(repository, builder, concrete.effects, boolean_features, numerical_features, diagnostics, data->effects);
                data->arguments.reserve(concrete.arguments.size());
                for (const auto& argument : concrete.arguments)
                    data->arguments.push_back(parse_do_argument(argument, concept_features, diagnostics));
                return intern_rule_variant(repository, builder, *data, symbol);
            }
            else if constexpr (std::same_as<RuleAst, ast::CallRule>)
            {
                auto data = runir::kr::ps::ext::checkout<Rule<CallTag>>(builder);
                data->source = source;
                data->target = target;
                append_conditions(repository, builder, concrete.conditions, boolean_features, numerical_features, diagnostics, data->conditions);
                if (const auto callee = find_module(modules, concrete.callee.text))
                    data->callee = *callee;
                else
                {
                    auto symbol_data = runir::kr::ps::ext::checkout<ModuleSymbol>(builder);
                    symbol_data->name = concrete.callee.text;
                    data->callee = intern(repository, *symbol_data).get_index();
                }
                data->arguments.reserve(concrete.arguments.size());
                for (const auto& argument : concrete.arguments)
                    data->arguments.push_back(
                        parse_call_argument(argument, concept_features, role_features, boolean_features, numerical_features, diagnostics));
                return intern_rule_variant(repository, builder, *data, symbol);
            }
        },
        rule.get());
}

}  // namespace

namespace
{

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>
parse_concept(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder,
              runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto result = runir::kr::dl::grammar::ast::ConceptConstructor<runir::kr::ExtFamilyTag> {};
    runir::kr::dl::grammar::parser::parse_concept_ast(description, result, error_handler);
    return parse_constructor(result, domain, ConstructorContext { repository, builder, diagnostics, nullptr });
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>
parse_role(const std::string& description,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
           runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder,
           runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto result = runir::kr::dl::grammar::ast::RoleConstructor<runir::kr::ExtFamilyTag> {};
    runir::kr::dl::grammar::parser::parse_role_ast(description, result, error_handler);
    return parse_constructor(result, domain, ConstructorContext { repository, builder, diagnostics, nullptr });
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>
parse_boolean(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder,
              runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto result = runir::kr::dl::grammar::ast::BooleanConstructor<runir::kr::ExtFamilyTag> {};
    runir::kr::dl::grammar::parser::parse_boolean_ast(description, result, error_handler);
    return parse_constructor(result, domain, ConstructorContext { repository, builder, diagnostics, nullptr });
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>
parse_numerical(const std::string& description,
                tyr::formalism::planning::DomainView domain,
                runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder,
                runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto result = runir::kr::dl::grammar::ast::NumericalConstructor<runir::kr::ExtFamilyTag> {};
    runir::kr::dl::grammar::parser::parse_numerical_ast(description, result, error_handler);
    return parse_constructor(result, domain, ConstructorContext { repository, builder, diagnostics, nullptr });
}

}  // namespace

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>
parse_concept(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto builder = runir::kr::dl::Builder<runir::kr::ExtFamilyTag> {};
    return parse_concept(description, domain, repository, builder, diagnostics);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>
parse_role(const std::string& description,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto builder = runir::kr::dl::Builder<runir::kr::ExtFamilyTag> {};
    return parse_role(description, domain, repository, builder, diagnostics);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>
parse_boolean(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto builder = runir::kr::dl::Builder<runir::kr::ExtFamilyTag> {};
    return parse_boolean(description, domain, repository, builder, diagnostics);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>
parse_numerical(const std::string& description,
                tyr::formalism::planning::DomainView domain,
                runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto builder = runir::kr::dl::Builder<runir::kr::ExtFamilyTag> {};
    return parse_numerical(description, domain, repository, builder, diagnostics);
}

ModuleView lower_module(const ast::Module& ast,
                        tyr::formalism::planning::DomainView domain,
                        Repository& repository,
                        Builders& builders,
                        runir::kr::parser::DiagnosticContext& diagnostics,
                        const std::unordered_map<std::string, ygg::Index<ModuleSymbol>>& module_symbols)
{
    validate_module_declarations(ast, diagnostics);
    auto references = ModuleReferences {};

    auto memory_states = std::unordered_map<std::string, ygg::Index<MemoryState>> {};
    for (const auto& state : ast.memory_states)
    {
        auto state_data = runir::kr::ps::ext::checkout<MemoryState>(builders.ps);
        state_data->name = state.value.text;
        if (!memory_states.emplace(state.value.text, intern(repository, *state_data).get_index()).second)
            diagnostics.throw_at(state.value, runir::kr::DuplicateDefinitionError("memory state", state.value.text));
    }

    const auto entry = memory_states.find(ast.entry.text);
    if (entry == memory_states.end())
        diagnostics.throw_at(ast.entry, runir::kr::UndefinedSymbolError("memory state", ast.entry.text));

    auto data = runir::kr::ps::ext::checkout<Module>(builders.ps);
    data->symbol = module_symbols.at(ast.name.text);
    data->entry_memory_state = entry->second;

    auto concept_argument = ygg::uint_t(0);
    auto role_argument = ygg::uint_t(0);
    auto boolean_argument = ygg::uint_t(0);
    auto numerical_argument = ygg::uint_t(0);
    for (const auto& argument : ast.arguments)
        append_argument(repository, builders.dl, *data, references, argument, concept_argument, role_argument, boolean_argument, numerical_argument);

    auto concept_register = ygg::uint_t(0);
    auto role_register = ygg::uint_t(0);
    for (const auto& reg : ast.registers)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                {
                    auto reg_data = runir::kr::dl::checkout<runir::kr::dl::Register<Category>>(builders.dl);
                    reg_data->name = concrete.symbol.text;
                    reg_data->identifier = runir::kr::dl::RegisterIdentifier<Category>(concept_register++);
                    const auto index = runir::kr::dl::get_or_create(repository.get_dl_repository(), *reg_data).first.get_index();
                    data->concept_registers.push_back(index);
                    references.concept_registers.by_name.emplace(concrete.symbol.text, index);
                    references.concept_registers.by_identifier.push_back(index);
                }
                else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
                {
                    auto reg_data = runir::kr::dl::checkout<runir::kr::dl::Register<Category>>(builders.dl);
                    reg_data->name = concrete.symbol.text;
                    reg_data->identifier = runir::kr::dl::RegisterIdentifier<Category>(role_register++);
                    const auto index = runir::kr::dl::get_or_create(repository.get_dl_repository(), *reg_data).first.get_index();
                    data->role_registers.push_back(index);
                    references.role_registers.by_name.emplace(concrete.symbol.text, index);
                    references.role_registers.by_identifier.push_back(index);
                }
            },
            reg.get());
    }

    for (const auto& state : ast.memory_states)
        data->memory_states.push_back(memory_states.at(state.value.text));

    auto concept_features = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>> {};
    auto role_features = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>> {};
    auto boolean_features = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>> {};
    auto numerical_features =
        std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>> {};
    for (const auto& feature : ast.features)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                append_feature(repository,
                               builders,
                               *data,
                               concrete,
                               domain,
                               concept_features,
                               role_features,
                               boolean_features,
                               numerical_features,
                               references,
                               diagnostics);
            },
            feature.get());
    }

    data->memory_transitions.reserve(ast.rule_entries.size());
    for (const auto& transition : ast.rule_entries)
    {
        data->memory_transitions.emplace_back();
        auto& parsed_transition = data->memory_transitions.back();
        parsed_transition.reserve(transition.rules.size());
        const auto source = require_memory_state(memory_states, transition.source, diagnostics);
        const auto target = require_memory_state(memory_states, transition.target, diagnostics);
        for (const auto& rule : transition.rules)
            parsed_transition.push_back(parse_rule(repository,
                                                   builders.ps,
                                                   rule,
                                                   source,
                                                   target,
                                                   domain,
                                                   module_symbols,
                                                   concept_features,
                                                   role_features,
                                                   boolean_features,
                                                   numerical_features,
                                                   references,
                                                   transition.symbol.text,
                                                   diagnostics)
                                            .get_index());
        ygg::canonicalize(parsed_transition);
    }

    return intern(repository, *data);
}

template<typename Function>
void with_diagnostic_scope(runir::kr::parser::DiagnosticContext& diagnostics, const runir::kr::parser::ErrorHandlerType* error_handler, Function&& function)
{
    if (error_handler)
    {
        auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, *error_handler);
        std::forward<Function>(function)();
    }
    else
    {
        std::forward<Function>(function)();
    }
}

void validate_module_set(const std::vector<ast::Module>& modules,
                         runir::kr::parser::DiagnosticContext& diagnostics,
                         const std::vector<std::unique_ptr<runir::kr::parser::ErrorHandlerType>>* error_handlers = nullptr)
{
    auto signatures_by_name = std::unordered_map<std::string, SignatureCounts> {};
    auto feature_symbols_by_module = std::unordered_map<std::string, FeatureSymbolSets> {};
    for (std::size_t i = 0; i < modules.size(); ++i)
    {
        const auto& module = modules[i];
        const auto* error_handler = error_handlers ? (*error_handlers)[i].get() : nullptr;
        with_diagnostic_scope(diagnostics,
                              error_handler,
                              [&]
                              {
                                  if (!signatures_by_name.emplace(module.name.text, signature_counts(module)).second)
                                      diagnostics.throw_at(module.name, runir::kr::DuplicateDefinitionError("module", module.name.text));
                                  feature_symbols_by_module.emplace(module.name.text, feature_symbols(module));
                              });
    }

    for (std::size_t i = 0; i < modules.size(); ++i)
    {
        const auto& module = modules[i];
        const auto* error_handler = error_handlers ? (*error_handlers)[i].get() : nullptr;
        with_diagnostic_scope(
            diagnostics,
            error_handler,
            [&]
            {
                const auto& caller_features = feature_symbols_by_module.at(module.name.text);
                for (const auto& transition : module.rule_entries)
                {
                    for (const auto& rule : transition.rules)
                    {
                        boost::apply_visitor(
                            [&](const auto& concrete)
                            {
                                using RuleAst = std::remove_cvref_t<decltype(concrete)>;
                                if constexpr (std::same_as<RuleAst, ast::CallRule>)
                                {
                                    const auto callee = signatures_by_name.find(concrete.callee.text);
                                    if (callee == signatures_by_name.end())
                                        diagnostics.throw_at(concrete.callee, runir::kr::UndefinedSymbolError("module", concrete.callee.text));

                                    const auto actual = call_argument_signature_counts(concrete, caller_features, diagnostics);
                                    if (actual != callee->second)
                                        diagnostics.throw_at(concrete,
                                                             runir::kr::InvalidExpressionError("Call from module " + module.name.text + " to module "
                                                                                               + concrete.callee.text + " has argument signature "
                                                                                               + signature_text(actual) + "; expected "
                                                                                               + signature_text(callee->second)));
                                }
                            },
                            rule.get());
                    }
                }
            });
    }
}

void validate_module_program(const ast::ModuleProgram& program, runir::kr::parser::DiagnosticContext& diagnostics)
{
    validate_module_set(program.modules, diagnostics);

    auto module_names = std::unordered_set<std::string> {};
    auto entry_has_arguments = false;
    for (const auto& module : program.modules)
    {
        module_names.emplace(module.name.text);
        if (module.name.text == program.entry.text)
            entry_has_arguments = !module.arguments.empty();
    }

    if (!module_names.contains(program.entry.text))
        diagnostics.throw_at(program.entry, runir::kr::UndefinedSymbolError("module", program.entry.text));
    if (entry_has_arguments)
        diagnostics.throw_at(program.entry, runir::kr::InvalidExpressionError("Module program entry must not declare formal arguments: " + program.entry.text));
}

ModuleView parse_module(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto ast = runir::kr::ps::ext::dl::ast::Module {};
    parser::parse_module_ast(description, ast, error_handler);
    auto dl_builder = runir::kr::dl::Builder<runir::kr::ExtFamilyTag> {};
    auto ps_builder = runir::kr::ps::ext::Builder {};
    auto builders = Builders { dl_builder, ps_builder };
    auto symbol_data = runir::kr::ps::ext::checkout<ModuleSymbol>(ps_builder);
    symbol_data->name = ast.name.text;
    auto module_symbols = std::unordered_map<std::string, ygg::Index<ModuleSymbol>> { { ast.name.text, intern(repository, *symbol_data).get_index() } };
    return lower_module(ast, domain, repository, builders, diagnostics, module_symbols);
}

ModuleProgramView parse_module_program(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto ast = runir::kr::ps::ext::dl::ast::ModuleProgram {};
    parser::parse_module_program_ast(description, ast, error_handler);
    validate_module_program(ast, diagnostics);

    auto dl_builder = runir::kr::dl::Builder<runir::kr::ExtFamilyTag> {};
    auto ps_builder = runir::kr::ps::ext::Builder {};
    auto builders = Builders { dl_builder, ps_builder };
    auto module_symbols = std::unordered_map<std::string, ygg::Index<ModuleSymbol>> {};
    for (const auto& module : ast.modules)
    {
        auto symbol_data = runir::kr::ps::ext::checkout<ModuleSymbol>(ps_builder);
        symbol_data->name = module.name.text;
        module_symbols.emplace(module.name.text, intern(repository, *symbol_data).get_index());
    }
    auto module_indices_by_name = std::unordered_map<std::string, ygg::Index<Module>> {};
    auto data = runir::kr::ps::ext::checkout<ModuleProgram>(ps_builder);
    data->modules.reserve(ast.modules.size());

    for (const auto& module : ast.modules)
    {
        auto view = lower_module(module, domain, repository, builders, diagnostics, module_symbols);
        data->modules.push_back(view.get_index());
        module_indices_by_name.emplace(module.name.text, view.get_index());
    }

    data->entry_module = module_indices_by_name.at(ast.entry.text);
    return intern(repository, *data);
}

std::vector<ModuleView> parse_modules(const std::vector<std::string>& descriptions, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto asts = std::vector<ast::Module>(descriptions.size());
    auto diagnostic_outputs = std::vector<std::ostringstream>(descriptions.size());
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto error_handlers = std::vector<std::unique_ptr<runir::kr::parser::ErrorHandlerType>> {};
    error_handlers.reserve(descriptions.size());

    for (std::size_t i = 0; i < descriptions.size(); ++i)
    {
        error_handlers.push_back(
            std::make_unique<runir::kr::parser::ErrorHandlerType>(descriptions[i].cbegin(), descriptions[i].cend(), diagnostic_outputs[i]));
        auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, *error_handlers.back());
        parser::parse_module_ast(descriptions[i], asts[i], *error_handlers.back());
    }

    validate_module_set(asts, diagnostics, &error_handlers);

    auto dl_builder = runir::kr::dl::Builder<runir::kr::ExtFamilyTag> {};
    auto ps_builder = runir::kr::ps::ext::Builder {};
    auto builders = Builders { dl_builder, ps_builder };
    auto result = std::vector<ModuleView> {};
    result.reserve(asts.size());
    auto module_symbols = std::unordered_map<std::string, ygg::Index<ModuleSymbol>> {};
    for (const auto& ast : asts)
    {
        auto symbol_data = runir::kr::ps::ext::checkout<ModuleSymbol>(ps_builder);
        symbol_data->name = ast.name.text;
        module_symbols.emplace(ast.name.text, intern(repository, *symbol_data).get_index());
    }
    for (std::size_t i = 0; i < asts.size(); ++i)
    {
        auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, *error_handlers[i]);
        auto view = lower_module(asts[i], domain, repository, builders, diagnostics, module_symbols);
        result.push_back(view);
    }
    return result;
}

}  // namespace runir::kr::ps::ext::dl
