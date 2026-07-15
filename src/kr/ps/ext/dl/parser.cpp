#include "runir/kr/ps/ext/dl/parser.hpp"

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
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <yggdrasil/containers/variant.hpp>

namespace runir::kr::ps::ext::dl
{
namespace
{
namespace dl_ = runir::kr::dl;
namespace dl_ast = runir::kr::dl::grammar::ast;
using DiagnosticContext = runir::kr::parser::DiagnosticContext;
template<typename Entity>
struct ReferenceTable
{
    std::unordered_map<std::string, ygg::Index<Entity>> by_name;
    std::vector<ygg::Index<Entity>> by_identifier;
};

struct ModuleReferences
{
    ReferenceTable<dl_::Argument<dl_::ConceptTag>> concept_arguments;
    ReferenceTable<dl_::Argument<dl_::RoleTag>> role_arguments;
    ReferenceTable<dl_::Argument<dl_::BooleanTag>> boolean_arguments;
    ReferenceTable<dl_::Argument<dl_::NumericalTag>> numerical_arguments;
    ReferenceTable<dl_::Register<dl_::ConceptTag>> concept_registers;
    ReferenceTable<dl_::Register<dl_::RoleTag>> role_registers;
};

struct ConstructorContext
{
    const DiagnosticContext& diagnostics;
    const ModuleReferences* references;

    template<typename Position, typename Error>
    [[noreturn]] void throw_at(const Position& position, Error error) const
    {
        diagnostics.throw_at(position, std::move(error));
    }
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
    if constexpr (requires { runir::kr::ps::ext::canonicalize(data); })
        runir::kr::ps::ext::canonicalize(data);
    return repository.get_or_create(data).first;
}

template<typename T>
auto intern_dl(runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository, ygg::Data<T>& data)
{
    if constexpr (requires { runir::kr::dl::canonicalize(data); })
        runir::kr::dl::canonicalize(data);
    return repository.get_or_create(data).first;
}

template<dl_::CategoryTag Category, typename T>
auto intern_constructor(runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository, ygg::Index<T> index)
{
    ygg::Data<dl_::Constructor<runir::kr::ExtFamilyTag, Category>> data(index);
    return intern_dl(repository, data);
}

auto parse_concept(const std::string& description,
                   tyr::formalism::planning::DomainView domain,
                   runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                   DiagnosticContext& diagnostics) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>;

auto parse_role(const std::string& description,
                tyr::formalism::planning::DomainView domain,
                runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                DiagnosticContext& diagnostics) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>;

auto parse_boolean(const std::string& description,
                   tyr::formalism::planning::DomainView domain,
                   runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                   DiagnosticContext& diagnostics) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>;

auto parse_numerical(const std::string& description,
                     tyr::formalism::planning::DomainView domain,
                     runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                     DiagnosticContext& diagnostics) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>;

template<dl_::CategoryTag Category>
auto parse_constructor(const dl_ast::Constructor<runir::kr::ExtFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                       const ConstructorContext& diagnostics) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>;

template<dl_::CategoryTag Category>
auto parse_constructor(const dl_ast::Constructor<runir::kr::ExtFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                       const DiagnosticContext& diagnostics,
                       const ModuleReferences* references = nullptr) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>;

template<dl_::CategoryTag Category>
auto parse_choice(const dl_ast::ConstructorOrNonTerminal<runir::kr::ExtFamilyTag, Category>& node,
                  tyr::formalism::planning::DomainView domain,
                  runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                  const ConstructorContext& diagnostics) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, dl_ast::NonTerminal<runir::kr::ExtFamilyTag, Category>>)
                diagnostics.throw_at(unwrapped.name,
                                     runir::kr::InvalidExpressionError("Ext module DL expressions cannot reference "
                                                                       "grammar nonterminals."));
            else
                return parse_constructor(unwrapped, domain, repository, diagnostics);
        },
        node.get());
}

template<tyr::formalism::FactKind T>
auto find_predicate(tyr::formalism::planning::DomainView domain, const std::string& name)
{
    for (auto predicate : domain.template get_predicates<T>())
        if (predicate.get_name() == name)
            return std::optional(predicate);
    return std::optional<tyr::formalism::planning::PredicateView<T>> {};
}

template<tyr::formalism::FactKind T, typename Position>
auto require_predicate(tyr::formalism::planning::DomainView domain,
                       const std::string& name,
                       size_t arity,
                       const char* constructor_name,
                       const Position& position,
                       const ConstructorContext& diagnostics)
{
    auto predicate = find_predicate<T>(domain, name);
    if (!predicate)
        return std::optional<ygg::Index<tyr::formalism::Predicate<T>>> {};

    if (predicate->get_arity() != arity)
        diagnostics.throw_at(position, runir::kr::ArityMismatchError(constructor_name, arity, predicate->get_arity()));

    return std::optional(predicate->get_index());
}

template<typename Position, typename Make>
auto resolve_predicate(tyr::formalism::planning::DomainView domain,
                       const std::string& name,
                       size_t arity,
                       const char* constructor_name,
                       const Position& position,
                       const ConstructorContext& diagnostics,
                       Make&& make)
{
    if (auto predicate = require_predicate<tyr::formalism::StaticTag>(domain, name, arity, constructor_name, position, diagnostics))
        return make(tyr::formalism::StaticTag {}, *predicate);
    if (auto predicate = require_predicate<tyr::formalism::FluentTag>(domain, name, arity, constructor_name, position, diagnostics))
        return make(tyr::formalism::FluentTag {}, *predicate);
    if (auto predicate = require_predicate<tyr::formalism::DerivedTag>(domain, name, arity, constructor_name, position, diagnostics))
        return make(tyr::formalism::DerivedTag {}, *predicate);

    diagnostics.throw_at(position, runir::kr::UndefinedSymbolError("predicate", name));
}

template<typename Entity, typename Identifier, typename Reference>
ygg::Index<Entity> resolve_reference(const Reference& reference,
                                     const ReferenceTable<Entity>* declarations,
                                     const char* kind,
                                     runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                                     const ConstructorContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& concrete) -> ygg::Index<Entity>
        {
            using Concrete = std::remove_cvref_t<decltype(concrete)>;
            if constexpr (std::same_as<Concrete, dl_ast::NumericReference>)
            {
                if (declarations)
                {
                    if (concrete.value >= declarations->by_identifier.size())
                        diagnostics.throw_at(concrete, runir::kr::UndefinedSymbolError(kind, std::to_string(concrete.value)));
                    return declarations->by_identifier[concrete.value];
                }

                try
                {
                    auto data = ygg::Data<Entity>(std::to_string(concrete.value), Identifier(concrete.value));
                    return intern_dl(repository, data).get_index();
                }
                catch (const std::out_of_range&)
                {
                    diagnostics.throw_at(concrete, runir::kr::UndefinedSymbolError(kind, std::to_string(concrete.value)));
                }
            }
            else
            {
                if (declarations)
                {
                    const auto it = declarations->by_name.find(concrete.text);
                    if (it != declarations->by_name.end())
                        return it->second;
                    diagnostics.throw_at(concrete, runir::kr::UndefinedSymbolError(kind, concrete.text));
                }
                diagnostics.throw_at(concrete, runir::kr::InvalidExpressionError("Expected numeric DL reference: " + concrete.text));
            }
        },
        reference.get());
}

auto require_object(tyr::formalism::planning::DomainView domain, const dl_ast::Identifier& name, const ConstructorContext& diagnostics)
{
    for (auto object : domain.get_constants())
        if (object.get_name() == name.text)
            return object.get_index();
    diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("constant", name.text));
}

auto require_objects(tyr::formalism::planning::DomainView domain, const std::vector<dl_ast::Identifier>& names, const ConstructorContext& diagnostics)
{
    auto result = ygg::IndexList<tyr::formalism::Object> {};
    result.reserve(names.size());
    for (const auto& name : names)
        result.push_back(require_object(domain, name, diagnostics));
    return result;
}

auto parse_dl(const dl_ast::ConceptBot<runir::kr::ExtFamilyTag>&,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext&)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::BotTag>> data;
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptTop<runir::kr::ExtFamilyTag>&,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext&)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::TopTag>> data;
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptAtomicState<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name.text,
                             1,
                             "ConceptAtomicState",
                             node.predicate_name,
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
                             });
}

auto parse_dl(const dl_ast::ConceptAtomicGoal<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name.text,
                             1,
                             "ConceptAtomicGoal",
                             node.predicate_name,
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_concept(const Ast& node,
                          tyr::formalism::planning::DomainView domain,
                          runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                          const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, Tag>> data(parse_choice(node.lhs, domain, repository, diagnostics).get_index(),
                                                               parse_choice(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptIntersection<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_binary_concept<dl_::IntersectionTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptUnion<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_binary_concept<dl_::UnionTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptValueRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_binary_concept<dl_::ValueRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptExistentialQuantification<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_binary_concept<dl_::ExistentialQuantificationTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptNegation<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::NegationTag>> data(parse_choice(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

template<typename Tag, typename Ast>
auto parse_number_restriction(const Ast& node,
                              tyr::formalism::planning::DomainView domain,
                              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, Tag>> data(node.n, parse_choice(node.role, domain, repository, diagnostics).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptAtLeastNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_number_restriction<dl_::AtLeastNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptAtMostNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_number_restriction<dl_::AtMostNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptExactNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_number_restriction<dl_::ExactNumberRestrictionTag>(node, domain, repository, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_qualified_number_restriction(const Ast& node,
                                        tyr::formalism::planning::DomainView domain,
                                        runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                                        const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, Tag>> data(node.n,
                                                               parse_choice(node.role, domain, repository, diagnostics).get_index(),
                                                               parse_choice(node.concept_, domain, repository, diagnostics).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_qualified_number_restriction<dl_::QualifiedAtLeastNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_qualified_number_restriction<dl_::QualifiedAtMostNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptQualifiedExactNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_qualified_number_restriction<dl_::QualifiedExactNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptRoleValueMap<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_binary_concept<dl_::RoleValueMapTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptAgreement<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_binary_concept<dl_::AgreementTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::ConceptRoleFillers<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::RoleFillersTag>> data(parse_choice(node.role, domain, repository, diagnostics).get_index(),
                                                                               require_objects(domain, node.object_names, diagnostics));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptOneOf<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::OneOfTag>> data(require_objects(domain, node.object_names, diagnostics));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptNominal<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::NominalTag>> data(require_object(domain, node.object_name, diagnostics));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptRegister<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::RegisterTag>> data(
        resolve_reference<dl_::Register<dl_::ConceptTag>, dl_::RegisterIdentifier<dl_::ConceptTag>>(
            node.reference,
            diagnostics.references ? &diagnostics.references->concept_registers : nullptr,
            "concept register",
            repository,
            diagnostics));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptArgument<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::ArgumentTag<dl_::ConceptTag>>> data(
        resolve_reference<dl_::Argument<dl_::ConceptTag>, dl_::ArgumentIdentifier<dl_::ConceptTag>>(
            node.reference,
            diagnostics.references ? &diagnostics.references->concept_arguments : nullptr,
            "concept argument",
            repository,
            diagnostics));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleUniversal<runir::kr::ExtFamilyTag>&,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext&)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::UniversalTag>> data;
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleAtomicState<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name.text,
                             2,
                             "RoleAtomicState",
                             node.predicate_name,
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
                             });
}

auto parse_dl(const dl_ast::RoleAtomicGoal<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name.text,
                             2,
                             "RoleAtomicGoal",
                             node.predicate_name,
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_role(const Ast& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                       const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, Tag>> data(parse_choice(node.lhs, domain, repository, diagnostics).get_index(),
                                                            parse_choice(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleIntersection<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_binary_role<dl_::IntersectionTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::RoleUnion<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_binary_role<dl_::UnionTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::RoleComposition<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_binary_role<dl_::CompositionTag>(node, domain, repository, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node,
                      tyr::formalism::planning::DomainView domain,
                      runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                      const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, Tag>> data(parse_choice(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleComplement<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_unary_role<dl_::ComplementTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::RoleInverse<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_unary_role<dl_::InverseTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::RoleTransitiveClosure<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_unary_role<dl_::TransitiveClosureTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::RoleReflexiveTransitiveClosure<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return parse_unary_role<dl_::ReflexiveTransitiveClosureTag>(node, domain, repository, diagnostics);
}

auto parse_dl(const dl_ast::RoleRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::RestrictionTag>> data(parse_choice(node.lhs, domain, repository, diagnostics).get_index(),
                                                                            parse_choice(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleIdentity<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::IdentityTag>> data(parse_choice(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleRegister<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::RegisterTag>> data(resolve_reference<dl_::Register<dl_::RoleTag>, dl_::RegisterIdentifier<dl_::RoleTag>>(
        node.reference,
        diagnostics.references ? &diagnostics.references->role_registers : nullptr,
        "role register",
        repository,
        diagnostics));
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleArgument<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::ArgumentTag<dl_::RoleTag>>> data(
        resolve_reference<dl_::Argument<dl_::RoleTag>, dl_::ArgumentIdentifier<dl_::RoleTag>>(node.reference,
                                                                                              diagnostics.references ? &diagnostics.references->role_arguments :
                                                                                                                       nullptr,
                                                                                              "role argument",
                                                                                              repository,
                                                                                              diagnostics));
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::BooleanAtomicState<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name.text,
                             0,
                             "BooleanAtomicState",
                             node.predicate_name,
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<dl_::Boolean<runir::kr::ExtFamilyTag, dl_::AtomicStateTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<dl_::BooleanTag>(repository, intern_dl(repository, data).get_index());
                             });
}

auto parse_dl(const dl_ast::BooleanAtomicGoal<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name.text,
                             0,
                             "BooleanAtomicGoal",
                             node.predicate_name,
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<dl_::Boolean<runir::kr::ExtFamilyTag, dl_::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<dl_::BooleanTag>(repository, intern_dl(repository, data).get_index());
                             });
}

auto parse_dl(const dl_ast::BooleanNonempty<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    using Data = ygg::Data<dl_::Boolean<runir::kr::ExtFamilyTag, dl_::NonemptyTag>>;
    const auto arg = boost::apply_visitor([&](const auto& value) -> typename Data::ConstructorVariant
                                          { return parse_choice(unwrap(value), domain, repository, diagnostics).get_index(); },
                                          node.arg.get());
    Data data(arg);
    return intern_constructor<dl_::BooleanTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::BooleanArgument<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Boolean<runir::kr::ExtFamilyTag, dl_::ArgumentTag<dl_::BooleanTag>>> data(
        resolve_reference<dl_::Argument<dl_::BooleanTag>, dl_::ArgumentIdentifier<dl_::BooleanTag>>(
            node.reference,
            diagnostics.references ? &diagnostics.references->boolean_arguments : nullptr,
            "boolean argument",
            repository,
            diagnostics));
    return intern_constructor<dl_::BooleanTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::NumericalCount<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    using Data = ygg::Data<dl_::Numerical<runir::kr::ExtFamilyTag, dl_::CountTag>>;
    const auto arg = boost::apply_visitor([&](const auto& value) -> typename Data::ConstructorVariant
                                          { return parse_choice(unwrap(value), domain, repository, diagnostics).get_index(); },
                                          node.arg.get());
    Data data(arg);
    return intern_constructor<dl_::NumericalTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::NumericalDistance<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Numerical<runir::kr::ExtFamilyTag, dl_::DistanceTag>> data(parse_choice(node.lhs, domain, repository, diagnostics).get_index(),
                                                                              parse_choice(node.mid, domain, repository, diagnostics).get_index(),
                                                                              parse_choice(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<dl_::NumericalTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::NumericalArgument<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              const ConstructorContext& diagnostics)
{
    ygg::Data<dl_::Numerical<runir::kr::ExtFamilyTag, dl_::ArgumentTag<dl_::NumericalTag>>> data(
        resolve_reference<dl_::Argument<dl_::NumericalTag>, dl_::ArgumentIdentifier<dl_::NumericalTag>>(
            node.reference,
            diagnostics.references ? &diagnostics.references->numerical_arguments : nullptr,
            "numerical argument",
            repository,
            diagnostics));
    return intern_constructor<dl_::NumericalTag>(repository, intern_dl(repository, data).get_index());
}

template<dl_::CategoryTag Category>
auto parse_constructor(const dl_ast::Constructor<runir::kr::ExtFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                       const ConstructorContext& diagnostics) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
{
    return boost::apply_visitor([&](const auto& arg) { return parse_dl(unwrap(arg), domain, repository, diagnostics); }, node.get());
}

template<dl_::CategoryTag Category>
auto parse_constructor(const dl_ast::Constructor<runir::kr::ExtFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                       const DiagnosticContext& diagnostics,
                       const ModuleReferences* references) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
{
    return parse_constructor(node, domain, repository, ConstructorContext { diagnostics, references });
}

template<typename T>
struct AstCategory
{
};

template<dl_::CategoryTag Category>
struct AstCategory<ast::Argument<Category>>
{
    using Type = Category;
};

template<dl_::CategoryTag Category>
struct AstCategory<ast::Register<Category>>
{
    using Type = Category;
};

template<dl_::CategoryTag Category>
struct AstCategory<ast::Feature<Category>>
{
    using Type = Category;
};

template<dl_::CategoryTag Category>
struct AstCategory<ast::LoadRule<Category>>
{
    using Type = Category;
};

template<dl_::CategoryTag Category>
auto intern_argument(Repository& repository, const ast::Argument<Category>& argument, ygg::uint_t identifier)
{
    auto data = ygg::Data<dl_::Argument<Category>>(argument.symbol.text, runir::kr::dl::ArgumentIdentifier<Category>(identifier));
    return intern_dl(repository.get_dl_repository(), data);
}

using ConceptFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>;
using RoleFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>>;
using BooleanFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>>;
using NumericalFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>;
using MemoryStateMap = std::unordered_map<std::string, ygg::Index<MemoryState>>;
using ModuleSymbolMap = std::unordered_map<std::string, ygg::Index<ModuleSymbol>>;
using ModuleMap = std::unordered_map<std::string, ygg::Index<Module>>;

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

template<dl_::CategoryTag Category>
void increment(SignatureCounts& counts)
{
    if constexpr (std::same_as<Category, dl_::ConceptTag>)
        ++counts.concepts;
    else if constexpr (std::same_as<Category, dl_::RoleTag>)
        ++counts.roles;
    else if constexpr (std::same_as<Category, dl_::BooleanTag>)
        ++counts.booleans;
    else if constexpr (std::same_as<Category, dl_::NumericalTag>)
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

template<dl_::CategoryTag Category>
void validate_unique_names(const std::vector<ast::Register<Category>>& declarations, const char* kind, const DiagnosticContext& diagnostics)
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

template<dl_::CategoryTag Category>
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

void validate_module_declarations(const ast::Module& module, const DiagnosticContext& diagnostics)
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
                if constexpr (std::same_as<Category, dl_::ConceptTag>)
                    add_argument(concept_arguments, concrete.symbol, "concept");
                else if constexpr (std::same_as<Category, dl_::RoleTag>)
                    add_argument(role_arguments, concrete.symbol, "role");
                else if constexpr (std::same_as<Category, dl_::BooleanTag>)
                    add_argument(boolean_arguments, concrete.symbol, "boolean");
                else if constexpr (std::same_as<Category, dl_::NumericalTag>)
                    add_argument(numerical_arguments, concrete.symbol, "numerical");
            },
            argument.get());
    }

    validate_unique_names(collect_registers<dl_::ConceptTag>(module.registers), "concept", diagnostics);
    validate_unique_names(collect_registers<dl_::RoleTag>(module.registers), "role", diagnostics);

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
            if constexpr (std::same_as<Category, dl_::ConceptTag>)
            {
                const auto index = intern_argument<Category>(repository, concrete, concept_identifier++).get_index();
                data.concept_arguments.push_back(index);
                references.concept_arguments.by_name.emplace(concrete.symbol.text, index);
                references.concept_arguments.by_identifier.push_back(index);
            }
            else if constexpr (std::same_as<Category, dl_::RoleTag>)
            {
                const auto index = intern_argument<Category>(repository, concrete, role_identifier++).get_index();
                data.role_arguments.push_back(index);
                references.role_arguments.by_name.emplace(concrete.symbol.text, index);
                references.role_arguments.by_identifier.push_back(index);
            }
            else if constexpr (std::same_as<Category, dl_::BooleanTag>)
            {
                const auto index = intern_argument<Category>(repository, concrete, boolean_identifier++).get_index();
                data.boolean_arguments.push_back(index);
                references.boolean_arguments.by_name.emplace(concrete.symbol.text, index);
                references.boolean_arguments.by_identifier.push_back(index);
            }
            else if constexpr (std::same_as<Category, dl_::NumericalTag>)
            {
                const auto index = intern_argument<Category>(repository, concrete, numerical_identifier++).get_index();
                data.numerical_arguments.push_back(index);
                references.numerical_arguments.by_name.emplace(concrete.symbol.text, index);
                references.numerical_arguments.by_identifier.push_back(index);
            }
        },
        argument.get());
}

template<typename FeatureTag, typename ConcreteFeatureTag>
auto intern_dl_feature(Repository& repository,
                       ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, ConcreteFeatureTag>> constructor,
                       const std::string& symbol)
{
    ygg::Data<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>> concrete_data(constructor, symbol);
    const auto concrete = intern(repository, concrete_data);
    ygg::Data<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>> feature_data(concrete.get_index());
    return intern(repository, feature_data);
}

template<dl_::CategoryTag Category>
void append_feature(Repository& repository,
                    ygg::Data<Module>& module_data,
                    const ast::Feature<Category>& feature,
                    tyr::formalism::planning::DomainView domain,
                    ConceptFeatureMap& concept_features,
                    RoleFeatureMap& role_features,
                    BooleanFeatureMap& boolean_features,
                    NumericalFeatureMap& numerical_features,
                    const ModuleReferences& references,
                    DiagnosticContext& diagnostics)
{
    if constexpr (std::same_as<Category, dl_::ConceptTag>)
    {
        const auto constructor = parse_constructor(feature.expression, domain, repository.get_dl_repository(), diagnostics, &references);
        const auto view = intern_dl_feature<runir::kr::dl::ConceptTag>(repository, constructor.get_index(), feature.symbol.text);
        concept_features.emplace(feature.symbol.text, view.get_index());
        module_data.concept_features.push_back(view.get_index());
    }
    else if constexpr (std::same_as<Category, dl_::RoleTag>)
    {
        const auto constructor = parse_constructor(feature.expression, domain, repository.get_dl_repository(), diagnostics, &references);
        const auto view = intern_dl_feature<runir::kr::dl::RoleTag>(repository, constructor.get_index(), feature.symbol.text);
        role_features.emplace(feature.symbol.text, view.get_index());
        module_data.role_features.push_back(view.get_index());
    }
    else if constexpr (std::same_as<Category, dl_::BooleanTag>)
    {
        const auto constructor = parse_constructor(feature.expression, domain, repository.get_dl_repository(), diagnostics, &references);
        const auto view = intern_dl_feature<runir::kr::ps::dl::BooleanFeature>(repository, constructor.get_index(), feature.symbol.text);
        boolean_features.emplace(feature.symbol.text, view.get_index());
        module_data.boolean_features.push_back(view.get_index());
    }
    else if constexpr (std::same_as<Category, dl_::NumericalTag>)
    {
        const auto constructor = parse_constructor(feature.expression, domain, repository.get_dl_repository(), diagnostics, &references);
        const auto view = intern_dl_feature<runir::kr::ps::dl::NumericalFeature>(repository, constructor.get_index(), feature.symbol.text);
        numerical_features.emplace(feature.symbol.text, view.get_index());
        module_data.numerical_features.push_back(view.get_index());
    }
}

template<typename FeatureTag>
auto require_feature(const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>>& features,
                     const ast::Identifier& name,
                     const DiagnosticContext& diagnostics)
{
    const auto it = features.find(name.text);
    if (it == features.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("feature", name.text));
    return it->second;
}

template<typename FeatureTag, typename ObservationTag>
auto make_condition(Repository& repository, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>> feature)
{
    ygg::Data<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>> concrete_data(feature);
    const auto concrete = intern(repository, concrete_data);
    ygg::Data<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>> concrete_variant_data(concrete.get_index());
    const auto concrete_variant = intern(repository, concrete_variant_data);
    ygg::Data<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> variant_data(concrete_variant.get_index());
    return intern(repository, variant_data);
}

template<typename FeatureTag, typename ObservationTag>
auto make_effect(Repository& repository, ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>> feature)
{
    ygg::Data<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>> concrete_data(feature);
    const auto concrete = intern(repository, concrete_data);
    ygg::Data<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>> concrete_variant_data(concrete.get_index());
    const auto concrete_variant = intern(repository, concrete_variant_data);
    ygg::Data<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> variant_data(concrete_variant.get_index());
    return intern(repository, variant_data);
}

auto parse_condition(Repository& repository,
                     const ast::Condition& condition,
                     const BooleanFeatureMap& boolean_features,
                     const NumericalFeatureMap& numerical_features,
                     const DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& observation)
        {
            using Observation = std::remove_cvref_t<decltype(observation)>;
            if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Positive>)
            {
                return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(
                    repository,
                    require_feature(boolean_features, condition.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Negative>)
            {
                return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(
                    repository,
                    require_feature(boolean_features, condition.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::EqualZero>)
            {
                return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>(
                    repository,
                    require_feature(numerical_features, condition.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::GreaterZero>)
            {
                return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>(
                    repository,
                    require_feature(numerical_features, condition.feature, diagnostics));
            }
        },
        condition.observation.get());
}

auto parse_effect(Repository& repository,
                  const ast::Effect& effect,
                  const BooleanFeatureMap& boolean_features,
                  const NumericalFeatureMap& numerical_features,
                  const DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& observation)
        {
            using Observation = std::remove_cvref_t<decltype(observation)>;
            if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Positive>)
            {
                return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(
                    repository,
                    require_feature(boolean_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Negative>)
            {
                return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(
                    repository,
                    require_feature(boolean_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Unchanged>)
            {
                if (boolean_features.contains(effect.feature.text))
                    return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>(
                        repository,
                        require_feature(boolean_features, effect.feature, diagnostics));
                return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>(
                    repository,
                    require_feature(numerical_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Increases>)
            {
                return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>(
                    repository,
                    require_feature(numerical_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Decreases>)
            {
                return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>(
                    repository,
                    require_feature(numerical_features, effect.feature, diagnostics));
            }
        },
        effect.observation.get());
}

auto parse_conditions(Repository& repository,
                      const std::vector<ast::Condition>& observations,
                      const BooleanFeatureMap& boolean_features,
                      const NumericalFeatureMap& numerical_features,
                      const DiagnosticContext& diagnostics)
{
    auto result = ygg::IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> {};
    result.reserve(observations.size());
    for (const auto& observation : observations)
        result.push_back(parse_condition(repository, observation, boolean_features, numerical_features, diagnostics).get_index());
    return result;
}

auto parse_effects(Repository& repository,
                   const std::vector<ast::Effect>& observations,
                   const BooleanFeatureMap& boolean_features,
                   const NumericalFeatureMap& numerical_features,
                   const DiagnosticContext& diagnostics)
{
    auto result = ygg::IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> {};
    result.reserve(observations.size());
    for (const auto& observation : observations)
        result.push_back(parse_effect(repository, observation, boolean_features, numerical_features, diagnostics).get_index());
    return result;
}

auto parse_do_argument(const ast::SymbolExpression& expression, const ConceptFeatureMap& concept_features, const DiagnosticContext& diagnostics)
{
    return require_feature(concept_features, expression.symbol, diagnostics);
}

auto parse_call_argument(const ast::SymbolExpression& expression,
                         const ConceptFeatureMap& concept_features,
                         const RoleFeatureMap& role_features,
                         const BooleanFeatureMap& boolean_features,
                         const NumericalFeatureMap& numerical_features,
                         const DiagnosticContext& diagnostics) -> CallArgument
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
                if constexpr (std::same_as<Category, dl_::ConceptTag>)
                    result.concepts.emplace(concrete.symbol.text);
                else if constexpr (std::same_as<Category, dl_::RoleTag>)
                    result.roles.emplace(concrete.symbol.text);
                else if constexpr (std::same_as<Category, dl_::BooleanTag>)
                    result.booleans.emplace(concrete.symbol.text);
                else if constexpr (std::same_as<Category, dl_::NumericalTag>)
                    result.numericals.emplace(concrete.symbol.text);
            },
            feature.get());
    }
    return result;
}

void increment_signature_count(SignatureCounts& counts, const FeatureSymbolSets& features, const ast::Identifier& symbol, const DiagnosticContext& diagnostics)
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

SignatureCounts call_argument_signature_counts(const ast::CallRule& rule, const FeatureSymbolSets& features, const DiagnosticContext& diagnostics)
{
    auto result = SignatureCounts {};
    for (const auto& argument : rule.arguments)
        increment_signature_count(result, features, argument.symbol, diagnostics);
    return result;
}

template<RuleKind Kind>
auto intern_rule_variant(Repository& repository, ygg::Data<Rule<Kind>>& data, const std::string& symbol)
{
    const auto rule = intern(repository, data);
    ygg::Data<RuleVariant> variant_data(symbol, rule.get_index());
    return intern(repository, variant_data);
}

auto require_memory_state(const MemoryStateMap& memory_states, const ast::Identifier& name, const DiagnosticContext& diagnostics)
{
    const auto it = memory_states.find(name.text);
    if (it == memory_states.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("memory state", name.text));
    return it->second;
}

template<typename RegisterMap>
auto require_register(const RegisterMap& registers, const ast::Identifier& name, const DiagnosticContext& diagnostics)
{
    const auto it = registers.find(name.text);
    if (it == registers.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("register", name.text));
    return it->second;
}

auto find_module(const ModuleSymbolMap& modules, const std::string& name)
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

void validate_do_action(tyr::formalism::planning::DomainView domain, const ast::DoRule& rule, const DiagnosticContext& diagnostics)
{
    const auto arity = find_action_arity(domain, rule.action.text);
    if (!arity)
        diagnostics.throw_at(rule.action, runir::kr::UndefinedSymbolError("action", rule.action.text));

    if (rule.arguments.size() != *arity)
        diagnostics.throw_at(rule.action, runir::kr::ArityMismatchError("action " + rule.action.text, *arity, rule.arguments.size()));
}

template<runir::kr::dl::CategoryTag Category>
auto parse_load_rule(Repository& repository,
                     const ast::LoadRule<Category>& rule,
                     ygg::Index<MemoryState> source,
                     ygg::Index<MemoryState> target,
                     const ConceptFeatureMap& concept_features,
                     const RoleFeatureMap& role_features,
                     const BooleanFeatureMap& boolean_features,
                     const NumericalFeatureMap& numerical_features,
                     const ModuleReferences& references,
                     const std::string& symbol,
                     DiagnosticContext& diagnostics)
{
    ygg::Data<Rule<LoadTag<Category>>> data;
    data.source = source;
    data.target = target;
    data.conditions = parse_conditions(repository, rule.conditions, boolean_features, numerical_features, diagnostics);
    if constexpr (std::same_as<Category, dl_::ConceptTag>)
    {
        data.feature = require_feature(concept_features, rule.feature, diagnostics);
        data.reg = require_register(references.concept_registers.by_name, rule.reg, diagnostics);
    }
    else
    {
        data.feature = require_feature(role_features, rule.feature, diagnostics);
        data.reg = require_register(references.role_registers.by_name, rule.reg, diagnostics);
    }
    return intern_rule_variant(repository, data, symbol);
}

auto parse_rule(Repository& repository,
                const ast::Rule& rule,
                ygg::Index<MemoryState> source,
                ygg::Index<MemoryState> target,
                tyr::formalism::planning::DomainView domain,
                const ModuleSymbolMap& modules,
                const ConceptFeatureMap& concept_features,
                const RoleFeatureMap& role_features,
                const BooleanFeatureMap& boolean_features,
                const NumericalFeatureMap& numerical_features,
                const ModuleReferences& references,
                const std::string& symbol,
                DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& concrete) -> RuleVariantView
        {
            using RuleAst = std::remove_cvref_t<decltype(concrete)>;
            if constexpr (requires { typename AstCategory<RuleAst>::Type; })
            {
                using Category = typename AstCategory<RuleAst>::Type;
                return parse_load_rule<Category>(repository,
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
                ygg::Data<Rule<SketchTag>> data;
                data.source = source;
                data.target = target;
                data.conditions = parse_conditions(repository, concrete.conditions, boolean_features, numerical_features, diagnostics);
                data.effects = parse_effects(repository, concrete.effects, boolean_features, numerical_features, diagnostics);
                return intern_rule_variant(repository, data, symbol);
            }
            else if constexpr (std::same_as<RuleAst, ast::DoRule>)
            {
                validate_do_action(domain, concrete, diagnostics);
                ygg::Data<Rule<DoTag>> data(concrete.action.text);
                data.source = source;
                data.target = target;
                data.conditions = parse_conditions(repository, concrete.conditions, boolean_features, numerical_features, diagnostics);
                data.effects = parse_effects(repository, concrete.effects, boolean_features, numerical_features, diagnostics);
                for (const auto& argument : concrete.arguments)
                    data.arguments.push_back(parse_do_argument(argument, concept_features, diagnostics));
                return intern_rule_variant(repository, data, symbol);
            }
            else if constexpr (std::same_as<RuleAst, ast::CallRule>)
            {
                ygg::Data<Rule<CallTag>> data;
                data.source = source;
                data.target = target;
                data.conditions = parse_conditions(repository, concrete.conditions, boolean_features, numerical_features, diagnostics);
                if (const auto callee = find_module(modules, concrete.callee.text))
                    data.callee = *callee;
                else
                {
                    auto symbol_data = ygg::Data<ModuleSymbol>(concrete.callee.text);
                    data.callee = intern(repository, symbol_data).get_index();
                }
                for (const auto& argument : concrete.arguments)
                    data.arguments.push_back(parse_call_argument(argument, concept_features, role_features, boolean_features, numerical_features, diagnostics));
                return intern_rule_variant(repository, data, symbol);
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
              DiagnosticContext& diagnostics)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto scope = DiagnosticContext::Scope(diagnostics, error_handler);
    auto result = dl_ast::ConceptConstructor<runir::kr::ExtFamilyTag> {};
    runir::kr::dl::grammar::parser::parse_concept_ast(description, result, error_handler);
    return parse_constructor(result, domain, repository, diagnostics);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>
parse_role(const std::string& description,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
           DiagnosticContext& diagnostics)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto scope = DiagnosticContext::Scope(diagnostics, error_handler);
    auto result = dl_ast::RoleConstructor<runir::kr::ExtFamilyTag> {};
    runir::kr::dl::grammar::parser::parse_role_ast(description, result, error_handler);
    return parse_constructor(result, domain, repository, diagnostics);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>
parse_boolean(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
              DiagnosticContext& diagnostics)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto scope = DiagnosticContext::Scope(diagnostics, error_handler);
    auto result = dl_ast::BooleanConstructor<runir::kr::ExtFamilyTag> {};
    runir::kr::dl::grammar::parser::parse_boolean_ast(description, result, error_handler);
    return parse_constructor(result, domain, repository, diagnostics);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>
parse_numerical(const std::string& description,
                tyr::formalism::planning::DomainView domain,
                runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                DiagnosticContext& diagnostics)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto scope = DiagnosticContext::Scope(diagnostics, error_handler);
    auto result = dl_ast::NumericalConstructor<runir::kr::ExtFamilyTag> {};
    runir::kr::dl::grammar::parser::parse_numerical_ast(description, result, error_handler);
    return parse_constructor(result, domain, repository, diagnostics);
}

}  // namespace

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>
parse_concept(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    auto diagnostics = DiagnosticContext {};
    return parse_concept(description, domain, repository, diagnostics);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>
parse_role(const std::string& description,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    auto diagnostics = DiagnosticContext {};
    return parse_role(description, domain, repository, diagnostics);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>
parse_boolean(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    auto diagnostics = DiagnosticContext {};
    return parse_boolean(description, domain, repository, diagnostics);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>
parse_numerical(const std::string& description,
                tyr::formalism::planning::DomainView domain,
                runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    auto diagnostics = DiagnosticContext {};
    return parse_numerical(description, domain, repository, diagnostics);
}

ModuleView lower_module(const ast::Module& ast,
                        tyr::formalism::planning::DomainView domain,
                        Repository& repository,
                        DiagnosticContext& diagnostics,
                        const ModuleSymbolMap& module_symbols)
{
    validate_module_declarations(ast, diagnostics);
    auto references = ModuleReferences {};

    auto memory_states = MemoryStateMap {};
    for (const auto& state : ast.memory_states)
    {
        auto state_data = ygg::Data<MemoryState>(state.value.text);
        if (!memory_states.emplace(state.value.text, intern(repository, state_data).get_index()).second)
            diagnostics.throw_at(state.value, runir::kr::DuplicateDefinitionError("memory state", state.value.text));
    }

    const auto entry = memory_states.find(ast.entry.text);
    if (entry == memory_states.end())
        diagnostics.throw_at(ast.entry, runir::kr::UndefinedSymbolError("memory state", ast.entry.text));

    auto data = ygg::Data<Module>(module_symbols.at(ast.name.text));
    data.entry_memory_state = entry->second;

    auto concept_argument = ygg::uint_t(0);
    auto role_argument = ygg::uint_t(0);
    auto boolean_argument = ygg::uint_t(0);
    auto numerical_argument = ygg::uint_t(0);
    for (const auto& argument : ast.arguments)
        append_argument(repository, data, references, argument, concept_argument, role_argument, boolean_argument, numerical_argument);

    auto concept_register = ygg::uint_t(0);
    auto role_register = ygg::uint_t(0);
    for (const auto& reg : ast.registers)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<Category, dl_::ConceptTag>)
                {
                    auto reg_data = ygg::Data<dl_::Register<Category>>(concrete.symbol.text, runir::kr::dl::RegisterIdentifier<Category>(concept_register++));
                    const auto index = intern_dl(repository.get_dl_repository(), reg_data).get_index();
                    data.concept_registers.push_back(index);
                    references.concept_registers.by_name.emplace(concrete.symbol.text, index);
                    references.concept_registers.by_identifier.push_back(index);
                }
                else if constexpr (std::same_as<Category, dl_::RoleTag>)
                {
                    auto reg_data = ygg::Data<dl_::Register<Category>>(concrete.symbol.text, runir::kr::dl::RegisterIdentifier<Category>(role_register++));
                    const auto index = intern_dl(repository.get_dl_repository(), reg_data).get_index();
                    data.role_registers.push_back(index);
                    references.role_registers.by_name.emplace(concrete.symbol.text, index);
                    references.role_registers.by_identifier.push_back(index);
                }
            },
            reg.get());
    }

    for (const auto& state : ast.memory_states)
        data.memory_states.push_back(memory_states.at(state.value.text));

    auto concept_features = ConceptFeatureMap {};
    auto role_features = RoleFeatureMap {};
    auto boolean_features = BooleanFeatureMap {};
    auto numerical_features = NumericalFeatureMap {};
    for (const auto& feature : ast.features)
    {
        boost::apply_visitor(
            [&](const auto& concrete) {
                append_feature(repository,
                               data,
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

    data.memory_transitions.reserve(ast.rule_entries.size());
    for (const auto& transition : ast.rule_entries)
    {
        auto parsed_transition = ygg::IndexList<RuleVariant> {};
        const auto source = require_memory_state(memory_states, transition.source, diagnostics);
        const auto target = require_memory_state(memory_states, transition.target, diagnostics);
        for (const auto& rule : transition.rules)
            parsed_transition.push_back(parse_rule(repository,
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
        data.memory_transitions.push_back(std::move(parsed_transition));
    }

    return intern(repository, data);
}

template<typename Function>
void with_diagnostic_scope(DiagnosticContext& diagnostics, const runir::kr::parser::ErrorHandlerType* error_handler, Function&& function)
{
    if (error_handler)
    {
        auto scope = DiagnosticContext::Scope(diagnostics, *error_handler);
        std::forward<Function>(function)();
    }
    else
    {
        std::forward<Function>(function)();
    }
}

void validate_module_set(const std::vector<ast::Module>& modules,
                         DiagnosticContext& diagnostics,
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

void validate_module_program(const ast::ModuleProgram& program, DiagnosticContext& diagnostics)
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
    auto diagnostics = DiagnosticContext {};
    auto scope = DiagnosticContext::Scope(diagnostics, error_handler);
    auto ast = runir::kr::ps::ext::dl::ast::Module {};
    parser::parse_module_ast(description, ast, error_handler);
    auto symbol_data = ygg::Data<ModuleSymbol>(ast.name.text);
    auto module_symbols = ModuleSymbolMap { { ast.name.text, intern(repository, symbol_data).get_index() } };
    return lower_module(ast, domain, repository, diagnostics, module_symbols);
}

ModuleProgramView parse_module_program(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto diagnostic_output = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), diagnostic_output);
    auto diagnostics = DiagnosticContext {};
    auto scope = DiagnosticContext::Scope(diagnostics, error_handler);
    auto ast = runir::kr::ps::ext::dl::ast::ModuleProgram {};
    parser::parse_module_program_ast(description, ast, error_handler);
    validate_module_program(ast, diagnostics);

    auto module_views = std::vector<ModuleView> {};
    module_views.reserve(ast.modules.size());
    auto module_symbols = ModuleSymbolMap {};
    for (const auto& module : ast.modules)
    {
        auto symbol_data = ygg::Data<ModuleSymbol>(module.name.text);
        module_symbols.emplace(module.name.text, intern(repository, symbol_data).get_index());
    }
    auto module_indices_by_name = ModuleMap {};

    for (const auto& module : ast.modules)
    {
        auto view = lower_module(module, domain, repository, diagnostics, module_symbols);
        module_views.push_back(view);
        module_indices_by_name.emplace(module.name.text, view.get_index());
    }

    auto data = ygg::Data<ModuleProgram> {};
    data.entry_module = module_indices_by_name.at(ast.entry.text);
    for (const auto module : module_views)
        data.modules.push_back(module.get_index());

    return intern(repository, data);
}

std::vector<ModuleView> parse_modules(const std::vector<std::string>& descriptions, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto asts = std::vector<ast::Module>(descriptions.size());
    auto diagnostic_outputs = std::vector<std::ostringstream>(descriptions.size());
    auto diagnostics = DiagnosticContext {};
    auto error_handlers = std::vector<std::unique_ptr<runir::kr::parser::ErrorHandlerType>> {};
    error_handlers.reserve(descriptions.size());

    for (std::size_t i = 0; i < descriptions.size(); ++i)
    {
        error_handlers.push_back(
            std::make_unique<runir::kr::parser::ErrorHandlerType>(descriptions[i].cbegin(), descriptions[i].cend(), diagnostic_outputs[i]));
        auto scope = DiagnosticContext::Scope(diagnostics, *error_handlers.back());
        parser::parse_module_ast(descriptions[i], asts[i], *error_handlers.back());
    }

    validate_module_set(asts, diagnostics, &error_handlers);

    auto result = std::vector<ModuleView> {};
    result.reserve(asts.size());
    auto module_symbols = ModuleSymbolMap {};
    for (const auto& ast : asts)
    {
        auto symbol_data = ygg::Data<ModuleSymbol>(ast.name.text);
        module_symbols.emplace(ast.name.text, intern(repository, symbol_data).get_index());
    }
    for (std::size_t i = 0; i < asts.size(); ++i)
    {
        auto scope = DiagnosticContext::Scope(diagnostics, *error_handlers[i]);
        auto view = lower_module(asts[i], domain, repository, diagnostics, module_symbols);
        result.push_back(view);
    }
    return result;
}

}  // namespace runir::kr::ps::ext::dl
