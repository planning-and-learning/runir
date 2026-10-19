#ifndef RUNIR_TESTS_KR_DL_FAMILY_TRAITS_FIXTURES_HPP_
#define RUNIR_TESTS_KR_DL_FAMILY_TRAITS_FIXTURES_HPP_

#include <concepts>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/generate.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/ext/execution_repository.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <runir/kr/ps/icp/execution_repository.hpp>
#include <runir/kr/ps/icp/execution_view.hpp>
#include <runir/kr/uns/repository.hpp>

namespace runir::tests
{

template<typename Repository, typename T>
concept InternsData = requires(Repository& repository, ygg::Data<T>& data) {
    { ygg::formalism::insert(repository, data) } -> std::same_as<std::pair<ygg::View<ygg::Index<T>, Repository>, bool>>;
};

template<typename Repository, typename T>
consteval bool rejects_symbol()
{
    return !ygg::formalism::SupportsSymbol<Repository, T> && !InternsData<Repository, T>
           && !requires(Repository& repository, ygg::Data<T>& data) { repository.insert(data); }
           && !requires(const Repository& repository, const ygg::Data<T>& data) { repository.find(data); }
           && !requires(const Repository& repository, ygg::Index<T> index) { repository[index]; }
           && !requires(const Repository& repository) { repository.template size<T>(); };
}

template<kr::dl::FamilyTag Family>
consteval bool constructor_interning_contracts()
{
    using Semantic = kr::dl::Concept<Family, kr::dl::TopTag>;
    using Grammar = kr::dl::grammar::Concept<Family, kr::dl::TopTag>;
    using Cnf = kr::dl::cnf_grammar::Concept<Family, kr::dl::TopTag>;
    using SemanticRepository = kr::dl::ConstructorRepositoryFor<Family>;
    using GrammarRepository = kr::dl::grammar::ConstructorRepositoryFor<Family>;
    using CnfRepository = kr::dl::cnf_grammar::ConstructorRepositoryFor<Family>;
    return InternsData<SemanticRepository, Semantic> && InternsData<GrammarRepository, Grammar> && InternsData<CnfRepository, Cnf>
           && rejects_symbol<SemanticRepository, Grammar>() && rejects_symbol<GrammarRepository, Cnf>() && rejects_symbol<CnfRepository, Semantic>();
}

}  // namespace runir::tests

#endif
