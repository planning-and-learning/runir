#include "fixtures.hpp"

#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <tyr/formalism/planning/parser.hpp>

namespace runir::tests
{
namespace
{

auto wrap_concept(runir::kr::dl::ConstructorRepository& repository, auto value)
{
    auto data = urd::data<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>(value.index());
    return repository.insert(data).first;
}

auto wrap_role(runir::kr::dl::ConstructorRepository& repository, auto role)
{
    auto data = urd::data<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>(role.index());
    return repository.insert(data).first;
}

}  // namespace

TEST(RunirTests, SyntacticComplexityRecursesIntoConceptChildren)
{
    const auto planning_domain = tyr::formalism::planning::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto repository = runir::kr::dl::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto& repo = *repository;

    auto universal_data = urd::data<runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::UniversalTag>>();
    const auto universal = wrap_role(repo, repo.insert(universal_data).first);
    auto inverse_data = urd::data<runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::InverseTag>>(universal.index());
    const auto inverse = wrap_role(repo, repo.insert(inverse_data).first);

    auto at_least_data = urd::data<runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::AtLeastNumberRestrictionTag>>(1, inverse.index());
    const auto at_least = repo.insert(at_least_data).first;
    EXPECT_EQ(runir::kr::dl::semantics::syntactic_complexity(at_least), 3);

    auto objects = urd::flat_vector<urd::index<tyr::formalism::Object>> {};
    objects.push_back(planning_domain.get_domain().get_constants().front().index());
    auto fillers_data = urd::data<runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::RoleFillersTag>>(inverse.index(), objects);
    const auto fillers = repo.insert(fillers_data).first;
    EXPECT_EQ(runir::kr::dl::semantics::syntactic_complexity(fillers), 3);

    const auto fillers_constructor = wrap_concept(repo, fillers);
    auto qualified_data =
        urd::data<runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>(1,
                                                                                                                              inverse.index(),
                                                                                                                              fillers_constructor.index());
    EXPECT_EQ(runir::kr::dl::semantics::syntactic_complexity(repo.insert(qualified_data).first), 6);
}

}  // namespace runir::tests
