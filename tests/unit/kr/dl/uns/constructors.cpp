#include "fixtures.hpp"

#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <tyr/formalism/planning/parser.hpp>

namespace runir::tests
{
namespace
{

namespace dl = runir::kr::dl;
namespace sem = runir::kr::dl::semantics;
using Uns = runir::kr::UnsFamilyTag;

auto wrap_concept(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, auto value)
{
    auto data = ygg::Data<dl::Constructor<Uns, dl::ConceptTag>>(value.get_index());
    return repository.get_or_create(data).first;
}

auto wrap_role(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, auto role)
{
    auto data = ygg::Data<dl::Constructor<Uns, dl::RoleTag>>(role.get_index());
    return repository.get_or_create(data).first;
}

}  // namespace

TEST(RunirTests, SyntacticComplexityRecursesIntoConceptChildren)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto repository = dl::ConstructorRepositoryFactoryFor<kr::UnsFamilyTag>().create(planning_domain.get_repository());
    auto& repo = *repository;

    auto universal_data = ygg::Data<dl::Role<Uns, dl::UniversalTag>>();
    const auto universal = wrap_role(repo, repo.get_or_create(universal_data).first);
    auto inverse_data = ygg::Data<dl::Role<Uns, dl::InverseTag>>(universal.get_index());
    const auto inverse = wrap_role(repo, repo.get_or_create(inverse_data).first);

    auto at_least_data = ygg::Data<dl::Concept<Uns, dl::AtLeastNumberRestrictionTag>>(1, inverse.get_index());
    const auto at_least = repo.get_or_create(at_least_data).first;
    EXPECT_EQ(sem::syntactic_complexity(at_least), 3);

    auto objects = ygg::IndexList<tyr::formalism::Object> {};
    objects.push_back(planning_domain.get_domain().get_constants().front().get_index());
    auto fillers_data = ygg::Data<dl::Concept<Uns, dl::RoleFillersTag>>(inverse.get_index(), objects);
    const auto fillers = repo.get_or_create(fillers_data).first;
    EXPECT_EQ(sem::syntactic_complexity(fillers), 3);

    const auto fillers_constructor = wrap_concept(repo, fillers);
    auto qualified_data = ygg::Data<dl::Concept<Uns, dl::QualifiedAtLeastNumberRestrictionTag>>(1, inverse.get_index(), fillers_constructor.get_index());
    EXPECT_EQ(sem::syntactic_complexity(repo.get_or_create(qualified_data).first), 6);
}

}  // namespace runir::tests
