#ifndef RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/grammar/role_data.hpp"
#include <yggdrasil/containers/variant.hpp>

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NonemptyTag>>
{
    using ConstructorVariant = ::ygg::IndexVariant<GrammarArgumentTypes<Family>>;
    using Arg = ConstructorVariant;
    Index<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NonemptyTag>> index;
    ConstructorVariant arg;
    Data() = default;
    explicit Data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<ConstructorVariant, C>;
    template<typename C>
    explicit Data(const ViewVariant<C>& arg_) :
        index(),
        arg(std::visit([](const auto& view) -> ConstructorVariant { return ConstructorVariant(view.get_index()); }, arg_))
    {
    }

    auto cista_members() noexcept { return std::tie(index, arg); }
    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<>
struct Data<runir::kr::dl::grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> :
    runir::kr::dl::ArgumentData<runir::kr::dl::grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::ArgumentData<runir::kr::dl::grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                             runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::ComparisonTag Tag>
struct Data<runir::kr::dl::grammar::Boolean<Family, Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::grammar::Boolean<Family, Tag>,
                              runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>,
                              runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Boolean<Family, Tag>,
                                           runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>,
                                           runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>, bool>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>, bool>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::LogicalBinaryTag Tag>
struct Data<runir::kr::dl::grammar::Boolean<Family, Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::grammar::Boolean<Family, Tag>,
                              runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>,
                              runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Boolean<Family, Tag>,
                                           runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>,
                                           runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NotTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NotTag>,
                             runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NotTag>,
                                          runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
