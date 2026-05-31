#ifndef RUNIR_CNF_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/data_helpers.hpp"

#include <yggdrasil/containers/variant.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::NonemptyTag>>
{
    using Arg = ::cista::offset::variant<ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>,
                                         ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>>;

    Index<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::NonemptyTag>> index;
    Arg arg;

    Data() = default;
    explicit Data(Arg arg_) : index(), arg(std::move(arg_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

}  // namespace ygg

#endif
