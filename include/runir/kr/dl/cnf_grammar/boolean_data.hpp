#ifndef RUNIR_CNF_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/data_helpers.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>

namespace urd
{

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NonemptyTag>>
{
    using Arg = urd::index_variant<boost::mp11::mp_transform<runir::kr::dl::cnf_grammar::NonTerminal, runir::kr::dl::ConceptOrRoleTags>>;

    urd::index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NonemptyTag>> index;
    Arg arg;

    data() = default;
    explicit data(Arg arg_) : index(), arg(std::move(arg_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg> {}; }
};

template<>
struct data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> :
    runir::kr::dl::ArgumentData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::ArgumentData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                             runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

template<runir::kr::dl::ComparisonTag Tag>
struct data<runir::kr::dl::cnf_grammar::Boolean<Tag>> :
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
struct data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::BooleanConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::BooleanConstantTag>, bool>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::BooleanConstantTag>, bool>;
    using Base::Base;
};

template<runir::kr::dl::LogicalBinaryTag Tag>
struct data<runir::kr::dl::cnf_grammar::Boolean<Tag>> :
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
struct data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NotTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NotTag>, runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NotTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

}  // namespace urd

#endif
