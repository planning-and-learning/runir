#include <concepts>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/grammar_data.hpp>
#include <runir/kr/dl/grammar/grammar_index.hpp>
#include <runir/kr/dl/grammar/grammar_view.hpp>
#include <yggdrasil/core/concepts.hpp>

namespace runir::tests
{

namespace
{

template<typename T, typename Repository>
concept IndexedDataView =
    std::constructible_from<ygg::Index<T>, ygg::uint_t> && ygg::Identifiable<ygg::Index<T>> && ygg::Identifiable<ygg::Data<T>>
    && ygg::Identifiable<ygg::View<ygg::Index<T>, Repository>> && std::totally_ordered<ygg::Index<T>> && std::totally_ordered<ygg::Data<T>>
    && std::totally_ordered<ygg::View<ygg::Index<T>, Repository>> && requires(ygg::Data<T>& data, const ygg::View<ygg::Index<T>, Repository>& view) {
           data.index;
           data.concept_start;
           data.role_start;
           data.boolean_start;
           data.numerical_start;
           data.concept_derivation_rules;
           data.role_derivation_rules;
           data.boolean_derivation_rules;
           data.numerical_derivation_rules;
           data.domain;
           data.clear();
           view.get_index();
           view.template get_start<kr::dl::ConceptTag>();
           view.template get_start<kr::dl::RoleTag>();
           view.template get_start<kr::dl::BooleanTag>();
           view.template get_start<kr::dl::NumericalTag>();
           view.template get_derivation_rules<kr::dl::ConceptTag>();
           view.template get_derivation_rules<kr::dl::RoleTag>();
           view.template get_derivation_rules<kr::dl::BooleanTag>();
           view.template get_derivation_rules<kr::dl::NumericalTag>();
           view.get_domain();
       };

template<typename Family, typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, kr::dl::grammar::ConstructorRepositoryFor<Family>> && ...);
}

static_assert(indexed_data_views<kr::BaseFamilyTag>(kr::dl::grammar::FamilyGrammarTypes<kr::BaseFamilyTag> {}));
static_assert(indexed_data_views<kr::ExtFamilyTag>(kr::dl::grammar::FamilyGrammarTypes<kr::ExtFamilyTag> {}));
static_assert(indexed_data_views<kr::UnsFamilyTag>(kr::dl::grammar::FamilyGrammarTypes<kr::UnsFamilyTag> {}));
static_assert(std::same_as<kr::dl::grammar::FamilyGrammarView<kr::BaseFamilyTag>,
                           ygg::View<ygg::Index<kr::dl::grammar::GrammarTag<kr::BaseFamilyTag>>, kr::dl::grammar::BaseConstructorRepository>>);
static_assert(std::same_as<kr::dl::grammar::FamilyGrammarView<kr::ExtFamilyTag>,
                           ygg::View<ygg::Index<kr::dl::grammar::GrammarTag<kr::ExtFamilyTag>>, kr::dl::grammar::ExtConstructorRepository>>);
static_assert(std::same_as<kr::dl::grammar::FamilyGrammarView<kr::UnsFamilyTag>,
                           ygg::View<ygg::Index<kr::dl::grammar::GrammarTag<kr::UnsFamilyTag>>, kr::dl::grammar::UnsConstructorRepository>>);
static_assert(std::same_as<kr::dl::grammar::FamilyGrammarView<kr::BaseFamilyTag>, kr::dl::grammar::BaseGrammarView>);
static_assert(std::same_as<kr::dl::grammar::FamilyGrammarView<kr::ExtFamilyTag>, kr::dl::grammar::ExtGrammarView>);
static_assert(std::same_as<kr::dl::grammar::FamilyGrammarView<kr::UnsFamilyTag>, kr::dl::grammar::UnsGrammarView>);

}  // namespace

}
