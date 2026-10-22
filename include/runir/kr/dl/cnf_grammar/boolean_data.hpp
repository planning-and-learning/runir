#ifndef RUNIR_CNF_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/data_helpers.hpp"
#include <yggdrasil/containers/variant.hpp>

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NonemptyTag>>
{
    using Arg = ::ygg::IndexVariant<::ygg::MapTypeListT<runir::kr::dl::cnf_grammar::NonTerminal, runir::kr::dl::ConceptOrRoleTags>>;

    Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NonemptyTag>> index;
    Arg arg;

    Data() = default;
    explicit Data(Arg arg_) : index(), arg(std::move(arg_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<Arg, C>;
    template<typename C>
    explicit Data(const ViewVariant<C>& arg_) : index(), arg(std::visit([](const auto& view) -> Arg { return Arg(view.get_index()); }, arg_))
    {
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::arg> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> :
    runir::kr::dl::ArgumentData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::ArgumentData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                             runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

template<runir::kr::dl::ComparisonTag Tag>
struct Data<runir::kr::dl::cnf_grammar::Boolean<Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::cnf_grammar::Boolean<Tag>,
                              runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::comparison_operand_t<Tag>>,
                              runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::comparison_operand_t<Tag>>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::cnf_grammar::Boolean<Tag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::comparison_operand_t<Tag>>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::comparison_operand_t<Tag>>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::BooleanConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::BooleanConstantTag>, bool>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::BooleanConstantTag>, bool>;
    using Base::Base;
};

template<runir::kr::dl::LogicalBinaryTag Tag>
struct Data<runir::kr::dl::cnf_grammar::Boolean<Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::cnf_grammar::Boolean<Tag>,
                              runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::BooleanTag>,
                              runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::cnf_grammar::Boolean<Tag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::BooleanTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NotTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NotTag>, runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NotTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
