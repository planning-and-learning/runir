#ifndef RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/grammar/role_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NonemptyTag>>
{
    using ConstructorVariant = ::cista::offset::variant<Index<GrammarConceptChoice<Family>>, Index<GrammarRoleChoice<Family>>>;
    using Arg = ConstructorVariant;
    Index<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NonemptyTag>> index;
    ConstructorVariant arg;
    Data() = default;
    explicit Data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}
    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
    }
    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

template<>
struct Data<runir::kr::dl::grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> :
    runir::kr::dl::semantics::ArgumentData<runir::kr::dl::grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                           runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>
{
    using Base =
        runir::kr::dl::semantics::ArgumentData<runir::kr::dl::grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                               runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::ComparisonTag Tag>
struct Data<runir::kr::dl::grammar::Boolean<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Boolean<Family, Tag>,
                                         runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>,
                                         runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Boolean<Family, Tag>,
                                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>,
                                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>> :
    runir::kr::dl::semantics::IdentifierData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>, bool>
{
    using Base = runir::kr::dl::semantics::IdentifierData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>, bool>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::LogicalBinaryTag Tag>
struct Data<runir::kr::dl::grammar::Boolean<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Boolean<Family, Tag>,
                                         runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>,
                                         runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Boolean<Family, Tag>,
                                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>,
                                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NotTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NotTag>,
                                        runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NotTag>,
                                                     runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
