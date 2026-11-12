#ifndef RUNIR_SRC_KR_PARSER_DATALOG_HPP_
#define RUNIR_SRC_KR_PARSER_DATALOG_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/parser/ast.hpp"
#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/ps/ext/dl/ast/ast.hpp"

#include <functional>
#include <optional>
#include <string>
#include <tyr/datalog/lifted/database/datalog/declarations.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/maps/flat_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/flat_set.hpp>
#include <urd/store.hpp>
#include <variant>

/// Names in policies: features that DL expressions and Datalog rules read, and the heads of the :datalog section.
namespace runir::kr::parser::ext_constructors
{
struct ModuleReferences;
}

namespace runir::kr::parser::datalog
{

using FeatureReference = std::variant<urd::index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>,
                                      urd::index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>,
                                      urd::index<runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>,
                                      urd::index<runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>>,
                                      urd::index<runir::kr::dl::Query<>>>;

struct HeadReference
{
    urd::index<runir::kr::dl::DatalogProgram> program;
    urd::index<tyr::datalog::database::datalog::Relation> relation;
};

/// Features by name, each parsed at its first reference: a feature that reaches itself through references is an error.
class FeatureResolver
{
public:
    explicit FeatureResolver(const DiagnosticContext& diagnostics) : m_diagnostics(&diagnostics) {}

    void declare(const ast::Identifier& name, std::function<FeatureReference()> parse);
    std::optional<FeatureReference> resolve(const ast::Identifier& name);

private:
    struct Entry
    {
        std::function<FeatureReference()> parse;
        std::optional<FeatureReference> value;
        bool in_progress = false;
    };
    const DiagnosticContext* m_diagnostics;
    urd::Store<urd::map<std::string, Entry>> m_entries;
    urd::Store<urd::sequence<std::string>> m_stack;
};

using DatalogRules = urd::Store<urd::sequence<runir::kr::ps::ext::dl::ast::DatalogRule>>;

/// The :datalog section of a module: rules that read domain predicates, goal atoms ((goal p t...)), the module's
/// registers and arguments, and features, built at the first q_head into one program per head that features read. A
/// rule may not read a feature that reads the section's heads.
class DatalogSection
{
public:
    DatalogSection(const DatalogRules* rules,
                   urd::Store<urd::set<std::string>, urd::sorted> read_heads,
                   tyr::formalism::planning::DomainView domain,
                   runir::kr::dl::ConstructorRepository& repository,
                   runir::kr::dl::Builder& builder,
                   FeatureResolver& features,
                   const ext_constructors::ModuleReferences& references,
                   const DiagnosticContext& diagnostics);

    [[nodiscard]] bool defines(const std::string& name) const { return m_arities.contains(name); }
    std::optional<HeadReference> head(const ast::Identifier& name);
    /// Builds the program if no feature read a head, so that every rule is checked; `reference` locates errors.
    void validate(const ast::Identifier& reference);

private:
    const DatalogRules* m_rules;
    urd::Store<urd::set<std::string>, urd::sorted> m_weighted;
    /// The heads that q_head constructors read anywhere in the module, the program's outputs.
    urd::Store<urd::set<std::string>, urd::sorted> m_read_heads;
    tyr::formalism::planning::DomainView m_domain;
    runir::kr::dl::ConstructorRepository* m_repository;
    runir::kr::dl::Builder* m_builder;
    FeatureResolver* m_features;
    const ext_constructors::ModuleReferences* m_references;
    const DiagnosticContext* m_diagnostics;
    urd::Store<urd::map<std::string, size_t>, urd::sorted> m_arities;
    bool m_building = false;
    bool m_built = false;
    /// Per head that a feature reads, its program (ProgramBuilder::finish).
    urd::Store<urd::map<std::string, urd::index<runir::kr::dl::DatalogProgram>>, urd::sorted> m_programs;
    urd::Store<urd::map<std::string, urd::index<tyr::datalog::database::datalog::Relation>>> m_heads;

    void build(const ast::Identifier& reference);
};

}  // namespace runir::kr::parser::datalog

#endif
