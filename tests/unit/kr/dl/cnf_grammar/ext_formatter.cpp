#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/cnf_grammar/canonicalization.hpp>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::tests
{

TEST(RunirTests, ExtCnfGrammarNumericReferencesFormatWithoutExternalNames)
{
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_repository);

    const auto format = [&]<typename T>(auto identifier)
    {
        auto data = ygg::Data<T>(identifier);
        kr::dl::cnf_grammar::canonicalize(data);
        return fmt::format("{}", repository->get_or_create(data).first);
    };

    using ConceptRegister = kr::dl::RegisterIdentifier<kr::dl::ConceptTag>;
    using RoleRegister = kr::dl::RegisterIdentifier<kr::dl::RoleTag>;
    using ConceptArgument = kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>;
    using RoleArgument = kr::dl::ArgumentIdentifier<kr::dl::RoleTag>;
    using BooleanArgument = kr::dl::ArgumentIdentifier<kr::dl::BooleanTag>;
    using NumericalArgument = kr::dl::ArgumentIdentifier<kr::dl::NumericalTag>;

    EXPECT_EQ((format.template operator()<kr::dl::cnf_grammar::Concept<kr::ExtFamilyTag, kr::dl::RegisterTag>>(ConceptRegister(1))), "(c_register 1)");
    EXPECT_EQ((format.template operator()<kr::dl::cnf_grammar::Role<kr::ExtFamilyTag, kr::dl::RegisterTag>>(RoleRegister(2))), "(r_register 2)");
    EXPECT_EQ((format.template operator()<kr::dl::cnf_grammar::Concept<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::ConceptTag>>>(ConceptArgument(3))),
              "(c_argument 3)");
    EXPECT_EQ((format.template operator()<kr::dl::cnf_grammar::Role<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::RoleTag>>>(RoleArgument(4))),
              "(r_argument 4)");
    EXPECT_EQ((format.template operator()<kr::dl::cnf_grammar::Boolean<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::BooleanTag>>>(BooleanArgument(5))),
              "(b_argument 5)");
    EXPECT_EQ((format.template operator()<kr::dl::cnf_grammar::Numerical<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::NumericalTag>>>(NumericalArgument(6))),
              "(n_argument 6)");
}

}  // namespace runir::tests
