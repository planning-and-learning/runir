#ifndef RUNIR_SRC_KR_PARSER_DATALOG_HPP_
#define RUNIR_SRC_KR_PARSER_DATALOG_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/parser/ast.hpp"
#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/ps/ext/dl/ast/ast.hpp"

#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tyr/formalism/planning/repository.hpp>
#include <unordered_map>
#include <variant>
#include <vector>
#include <yggdrasil/database/datalog/declarations.hpp>

/// Names in policies: features that DL expressions and Datalog rules read, and the heads of the :datalog section.
namespace runir::kr::parser::ext_constructors
{
struct ModuleReferences;
}

namespace runir::kr::parser::datalog
{

using FeatureReference = std::variant<ygg::Index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>,
                                      ygg::Index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>,
                                      ygg::Index<runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>,
                                      ygg::Index<runir::kr::dl::Constructor<runir::kr::dl::NumericalTag>>,
                                      ygg::Index<runir::kr::dl::Query<>>>;

struct HeadReference
{
    ygg::Index<runir::kr::dl::DatalogProgram> program;
    ygg::Index<ygg::database::datalog::Relation> relation;
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
    std::unordered_map<std::string, Entry> m_entries;
    std::vector<std::string> m_stack;
};

using DatalogAst = runir::kr::ps::ext::dl::ast::DatalogProgram;

/// The :datalog section of a module: one program whose rules read domain predicates, goal atoms ((goal p t...)), the
/// module's concept and role registers and arguments, and concept, role and query features, built at the first q_head.
/// A rule may not read a feature that reads the program's heads.
class DatalogSection
{
public:
    DatalogSection(const DatalogAst* section,
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
    const DatalogAst* m_section;
    std::set<std::string> m_weighted;
    tyr::formalism::planning::DomainView m_domain;
    runir::kr::dl::ConstructorRepository* m_repository;
    runir::kr::dl::Builder* m_builder;
    FeatureResolver* m_features;
    const ext_constructors::ModuleReferences* m_references;
    const DiagnosticContext* m_diagnostics;
    std::map<std::string, size_t> m_arities;
    bool m_building = false;
    std::optional<ygg::Index<runir::kr::dl::DatalogProgram>> m_program;
    std::unordered_map<std::string, ygg::Index<ygg::database::datalog::Relation>> m_heads;

    void build(const ast::Identifier& reference);
};

}  // namespace runir::kr::parser::datalog

#endif
