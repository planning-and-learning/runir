#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/uns/repository.hpp>
#include <type_traits>

namespace runir::tests
{
namespace
{

using IcpFactory = kr::ps::icp::RepositoryFactory;
using ExtConstructors = kr::dl::ConstructorRepositoryPtrFor<kr::ExtFamilyTag>;
using BaseConstructors = kr::dl::ConstructorRepositoryPtrFor<kr::BaseFamilyTag>;
using UnsConstructors = kr::dl::ConstructorRepositoryPtrFor<kr::UnsFamilyTag>;

// ICP shares Ext expressions, while policy indices remain distinct.
static_assert(std::same_as<kr::ps::icp::Repository::DlRepositoryPtr, ExtConstructors>);
static_assert(std::same_as<kr::ps::base::Repository::DlRepositoryPtr, BaseConstructors>);
static_assert(std::same_as<kr::uns::Repository::DlRepositoryPtr, UnsConstructors>);
static_assert(std::is_invocable_v<decltype(&IcpFactory::create), IcpFactory&, ExtConstructors>);
static_assert(!std::is_invocable_v<decltype(&IcpFactory::create), IcpFactory&, BaseConstructors>);
static_assert(!std::is_invocable_v<decltype(&IcpFactory::create), IcpFactory&, UnsConstructors>);
static_assert(
    !std::same_as<ygg::Index<kr::ps::Feature<kr::IcpFamilyTag, kr::dl::ConceptTag>>, ygg::Index<kr::ps::Feature<kr::ExtFamilyTag, kr::dl::ConceptTag>>>);

using IcpQuery = kr::ps::ConcreteFeature<kr::IcpFamilyTag, kr::DlTag, kr::ps::dl::QueryFeature>;
static_assert(std::same_as<typename ygg::Data<IcpQuery>::Expression, kr::dl::Query<kr::ExtFamilyTag>>);

// Classifiers expose Boolean features, without acquiring policy conditions or effects.
static_assert(std::same_as<kr::ps::PsConditionTypes<kr::UnsFamilyTag>, ygg::TypeList<>>);
static_assert(std::same_as<kr::ps::PsEffectTypes<kr::UnsFamilyTag>, ygg::TypeList<>>);

}  // namespace
}  // namespace runir::tests
