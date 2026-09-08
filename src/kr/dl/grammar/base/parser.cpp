#include "runir/kr/dl/grammar/parser.hpp"

#include "kr/parser/resolution.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/canonicalization.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/parser/parser.hpp"
#include "runir/kr/parser/diagnostics.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <cista/containers/variant.h>
#include <sstream>
#include <string>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>

namespace runir::kr::dl::grammar
{
namespace
{

struct ConstructorContext
{
    ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository;
    Builder<runir::kr::BaseFamilyTag>& builder;
    const runir::kr::parser::DiagnosticContext& diagnostics;
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
}  // namespace runir::kr::dl::grammar

template<typename T, typename Initialize>
auto intern(const ConstructorContext& context, Initialize&& initialize)
{
    auto data = runir::kr::dl::grammar::checkout<T>(context.builder);
    std::forward<Initialize>(initialize)(*data);
    return runir::kr::dl::grammar::get_or_create(context.repository, *data).first;
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(const ConstructorContext& context, ygg::Index<T> index)
{
    return intern<Constructor<Family, Category>>(context, [&](auto& data) { data.variant = index; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename T, typename Initialize>
auto intern_concrete(const ConstructorContext& context, Initialize&& initialize)
{
    return intern_constructor<Family, Category>(context, intern<T>(context, std::forward<Initialize>(initialize)).get_index());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse_constructor(const ast::Constructor<Family, Category>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context);

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::NonTerminal<Family, Category>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return intern<NonTerminal<Family, Category>>(context, [&](auto& data) { data.name = node.name.text; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse_constructor_or_non_terminal(const ast::ConstructorOrNonTerminal<Family, Category>& node,
                                       tyr::formalism::planning::DomainView domain,
                                       const ConstructorContext& context)
{
    const auto index = boost::apply_visitor(
        [&](const auto& arg) -> ygg::Data<ConstructorOrNonTerminal<Family, Category>>::Variant
        {
            const auto& unwrapped = unwrap(arg);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, ast::NonTerminal<Family, Category>>)
                return parse(unwrapped, domain, context).get_index();
            else
                return parse_constructor(unwrapped, domain, context).get_index();
        },
        node.get());

    return intern<ConstructorOrNonTerminal<Family, Category>>(context, [&](auto& data) { data.variant = index; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptBot<Family>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    const auto concrete = intern<Concept<Family, BotTag>>(context, [](auto&) {});
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptTop<Family>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    const auto concrete = intern<Concept<Family, TopTag>>(context, [](auto&) {});
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                1,
                                                "ConceptAtomicState",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    const auto concrete = intern<Concept<Family, AtomicStateTag<T>>>(context,
                                                                                                                     [&](auto& data)
                                                                                                                     {
                                                                                                                         data.predicate = predicate;
                                                                                                                         data.polarity = true;
                                                                                                                     });
                                                    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
                                                });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                1,
                                                "ConceptAtomicGoal",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    const auto concrete = intern<Concept<Family, AtomicGoalTag<T>>>(context,
                                                                                                                    [&](auto& data)
                                                                                                                    {
                                                                                                                        data.predicate = predicate;
                                                                                                                        data.polarity = node.polarity;
                                                                                                                    });
                                                    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
                                                });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptIntersection<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    const auto concrete = intern<Concept<Family, IntersectionTag>>(context,
                                                                   [&](auto& data)
                                                                   {
                                                                       data.lhs = lhs;
                                                                       data.rhs = rhs;
                                                                   });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptUnion<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    const auto concrete = intern<Concept<Family, UnionTag>>(context,
                                                            [&](auto& data)
                                                            {
                                                                data.lhs = lhs;
                                                                data.rhs = rhs;
                                                            });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptNegation<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context).get_index();
    const auto concrete = intern<Concept<Family, NegationTag>>(context, [&](auto& data) { data.arg = arg; });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptValueRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    const auto concrete = intern<Concept<Family, ValueRestrictionTag>>(context,
                                                                       [&](auto& data)
                                                                       {
                                                                           data.lhs = lhs;
                                                                           data.rhs = rhs;
                                                                       });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptExistentialQuantification<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    const auto concrete = intern<Concept<Family, ExistentialQuantificationTag>>(context,
                                                                                [&](auto& data)
                                                                                {
                                                                                    data.lhs = lhs;
                                                                                    data.rhs = rhs;
                                                                                });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtLeastNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context).get_index();
    const auto concrete = intern<Concept<Family, AtLeastNumberRestrictionTag>>(context,
                                                                               [&](auto& data)
                                                                               {
                                                                                   data.n = node.n;
                                                                                   data.role = role;
                                                                               });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtMostNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context).get_index();
    const auto concrete = intern<Concept<Family, AtMostNumberRestrictionTag>>(context,
                                                                              [&](auto& data)
                                                                              {
                                                                                  data.n = node.n;
                                                                                  data.role = role;
                                                                              });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptExactNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context).get_index();
    const auto concrete = intern<Concept<Family, ExactNumberRestrictionTag>>(context,
                                                                             [&](auto& data)
                                                                             {
                                                                                 data.n = node.n;
                                                                                 data.role = role;
                                                                             });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedAtLeastNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context).get_index();
    const auto concept_index = parse_constructor_or_non_terminal(node.concept_, domain, context).get_index();
    const auto concrete = intern<Concept<Family, QualifiedAtLeastNumberRestrictionTag>>(context,
                                                                                        [&](auto& data)
                                                                                        {
                                                                                            data.n = node.n;
                                                                                            data.role = role;
                                                                                            data.concept_ = concept_index;
                                                                                        });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedAtMostNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context).get_index();
    const auto concept_index = parse_constructor_or_non_terminal(node.concept_, domain, context).get_index();
    const auto concrete = intern<Concept<Family, QualifiedAtMostNumberRestrictionTag>>(context,
                                                                                       [&](auto& data)
                                                                                       {
                                                                                           data.n = node.n;
                                                                                           data.role = role;
                                                                                           data.concept_ = concept_index;
                                                                                       });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedExactNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context).get_index();
    const auto concept_index = parse_constructor_or_non_terminal(node.concept_, domain, context).get_index();
    const auto concrete = intern<Concept<Family, QualifiedExactNumberRestrictionTag>>(context,
                                                                                      [&](auto& data)
                                                                                      {
                                                                                          data.n = node.n;
                                                                                          data.role = role;
                                                                                          data.concept_ = concept_index;
                                                                                      });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptRoleValueMap<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    const auto concrete = intern<Concept<Family, RoleValueMapTag>>(context,
                                                                   [&](auto& data)
                                                                   {
                                                                       data.lhs = lhs;
                                                                       data.rhs = rhs;
                                                                   });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAgreement<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    const auto concrete = intern<Concept<Family, AgreementTag>>(context,
                                                                [&](auto& data)
                                                                {
                                                                    data.lhs = lhs;
                                                                    data.rhs = rhs;
                                                                });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptRoleFillers<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context).get_index();
    const auto concrete =
        intern<Concept<Family, RoleFillersTag>>(context,
                                                [&](auto& data)
                                                {
                                                    data.role = role;
                                                    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data.objects);
                                                });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptOneOf<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto concrete =
        intern<Concept<Family, OneOfTag>>(context,
                                          [&](auto& data) { runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data.objects); });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptNominal<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto object = runir::kr::parser::require_object(domain, node.object_name, context.diagnostics);
    const auto concrete = intern<Concept<Family, NominalTag>>(context, [&](auto& data) { data.object = object; });
    return intern_constructor<Family, ConceptTag>(context, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleUniversal<Family>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return intern_concrete<Family, RoleTag, Role<Family, UniversalTag>>(context, [](auto&) {});
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                2,
                                                "RoleAtomicState",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    return intern_concrete<Family, RoleTag, Role<Family, AtomicStateTag<T>>>(context,
                                                                                                                             [&](auto& data)
                                                                                                                             {
                                                                                                                                 data.predicate = predicate;
                                                                                                                                 data.polarity = true;
                                                                                                                             });
                                                });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                2,
                                                "RoleAtomicGoal",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    return intern_concrete<Family, RoleTag, Role<Family, AtomicGoalTag<T>>>(context,
                                                                                                                            [&](auto& data)
                                                                                                                            {
                                                                                                                                data.predicate = predicate;
                                                                                                                                data.polarity = node.polarity;
                                                                                                                            });
                                                });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleIntersection<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, IntersectionTag>>(context,
                                                                           [&](auto& data)
                                                                           {
                                                                               data.lhs = lhs;
                                                                               data.rhs = rhs;
                                                                           });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleUnion<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, UnionTag>>(context,
                                                                    [&](auto& data)
                                                                    {
                                                                        data.lhs = lhs;
                                                                        data.rhs = rhs;
                                                                    });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleComplement<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, ComplementTag>>(context, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleInverse<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, InverseTag>>(context, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleComposition<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, CompositionTag>>(context,
                                                                          [&](auto& data)
                                                                          {
                                                                              data.lhs = lhs;
                                                                              data.rhs = rhs;
                                                                          });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleTransitiveClosure<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, TransitiveClosureTag>>(context, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleReflexiveTransitiveClosure<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, ReflexiveTransitiveClosureTag>>(context, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, RestrictionTag>>(context,
                                                                          [&](auto& data)
                                                                          {
                                                                              data.lhs = lhs;
                                                                              data.rhs = rhs;
                                                                          });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleIdentity<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, IdentityTag>>(context, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                0,
                                                "BooleanAtomicState",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    return intern_concrete<Family, BooleanTag, Boolean<Family, AtomicStateTag<T>>>(context,
                                                                                                                                   [&](auto& data)
                                                                                                                                   {
                                                                                                                                       data.predicate =
                                                                                                                                           predicate;
                                                                                                                                       data.polarity =
                                                                                                                                           node.polarity;
                                                                                                                                   });
                                                });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                0,
                                                "BooleanAtomicGoal",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    return intern_concrete<Family, BooleanTag, Boolean<Family, AtomicGoalTag<T>>>(context,
                                                                                                                                  [&](auto& data)
                                                                                                                                  {
                                                                                                                                      data.predicate =
                                                                                                                                          predicate;
                                                                                                                                      data.polarity =
                                                                                                                                          node.polarity;
                                                                                                                                  });
                                                });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanNonempty<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = boost::apply_visitor([&](const auto& value) -> ygg::Data<Boolean<Family, NonemptyTag>>::Arg
                                          { return parse_constructor_or_non_terminal(unwrap(value), domain, context).get_index(); },
                                          node.arg.get());

    return intern_concrete<Family, BooleanTag, Boolean<Family, NonemptyTag>>(context, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::NumericalCount<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = boost::apply_visitor([&](const auto& value) -> ygg::Data<Numerical<Family, CountTag>>::Arg
                                          { return parse_constructor_or_non_terminal(unwrap(value), domain, context).get_index(); },
                                          node.arg.get());

    return intern_concrete<Family, NumericalTag, Numerical<Family, CountTag>>(context, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::NumericalDistance<Family>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context).get_index();
    const auto mid = parse_constructor_or_non_terminal(node.mid, domain, context).get_index();
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context).get_index();
    return intern_concrete<Family, NumericalTag, Numerical<Family, DistanceTag>>(context,
                                                                                 [&](auto& data)
                                                                                 {
                                                                                     data.lhs = lhs;
                                                                                     data.mid = mid;
                                                                                     data.rhs = rhs;
                                                                                 });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse_constructor(const ast::Constructor<Family, Category>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, context); }, node.get());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::DerivationRule<Family, Category>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    return intern<DerivationRule<Family, Category>>(context,
                                                    [&](auto& data)
                                                    {
                                                        for (const auto& symbol : node.rhs)
                                                            data.rhs.push_back(parse_constructor_or_non_terminal(symbol, domain, context).get_index());
                                                        data.lhs = parse(node.lhs, domain, context).get_index();
                                                    });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void append_derivation_rule(ygg::IndexList<DerivationRule<Family, Category>>& rules,
                            const ast::DerivationRule<Family, Category>& node,
                            tyr::formalism::planning::DomainView domain,
                            const ConstructorContext& context)
{
    rules.push_back(parse(node, domain, context).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
void append_derivation_rule(ygg::Data<GrammarTag<Family>>& data,
                            const ast::DerivationRuleVariant<Family>& node,
                            tyr::formalism::planning::DomainView domain,
                            const ConstructorContext& context)
{
    boost::apply_visitor(
        [&](const auto& value)
        {
            const auto& rule = unwrap(value);
            using Category = typename std::decay_t<decltype(rule)>::CategoryTag;
            if constexpr (std::same_as<Category, ConceptTag>)
                append_derivation_rule(data.concept_derivation_rules, rule, domain, context);
            else if constexpr (std::same_as<Category, RoleTag>)
                append_derivation_rule(data.role_derivation_rules, rule, domain, context);
            else if constexpr (std::same_as<Category, BooleanTag>)
                append_derivation_rule(data.boolean_derivation_rules, rule, domain, context);
            else if constexpr (std::same_as<Category, NumericalTag>)
                append_derivation_rule(data.numerical_derivation_rules, rule, domain, context);
        },
        node.get());
}

template<runir::kr::dl::CategoryTag Category>
auto fixed_start_name()
{
    if constexpr (std::same_as<Category, ConceptTag>)
        return "c_0";
    else if constexpr (std::same_as<Category, RoleTag>)
        return "r_0";
    else if constexpr (std::same_as<Category, BooleanTag>)
        return "b_0";
    else if constexpr (std::same_as<Category, NumericalTag>)
        return "n_0";
}

template<runir::kr::dl::CategoryTag Category>
auto parse_fixed_start(tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    ast::NonTerminal<runir::kr::BaseFamilyTag, Category> start;
    start.name.text = fixed_start_name<Category>();
    return parse(start, domain, context).get_index();
}

auto parse_grammar_impl(const std::string& description,
                        tyr::formalism::planning::DomainView domain,
                        ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository)
{
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    auto ast = runir::kr::dl::grammar::ast::Grammar<runir::kr::BaseFamilyTag> {};
    parser::parse_grammar_ast(description, ast, error_handler);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    const auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto builder = Builder<runir::kr::BaseFamilyTag> {};
    const auto context = ConstructorContext { repository, builder, diagnostics };

    auto data = runir::kr::dl::grammar::checkout<GrammarTag<runir::kr::BaseFamilyTag>>(builder);

    data->concept_start = parse_fixed_start<ConceptTag>(domain, context);
    data->role_start = parse_fixed_start<RoleTag>(domain, context);
    data->boolean_start = parse_fixed_start<BooleanTag>(domain, context);
    data->numerical_start = parse_fixed_start<NumericalTag>(domain, context);

    for (const auto& rule : ast.rules)
        append_derivation_rule(*data, rule, domain, context);

    data->domain = domain.get_index();
    return runir::kr::dl::grammar::get_or_create(repository, *data).first;
}

}  // namespace

FamilyGrammarView<runir::kr::BaseFamilyTag>
parse_grammar(const std::string& description, tyr::formalism::planning::DomainView domain, ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository)
{
    return parse_grammar_impl(description, domain, repository);
}

}  // namespace runir::kr::dl::grammar
