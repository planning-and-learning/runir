#ifndef RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/knowledge_representation/dl/grammar/data_helpers.hpp"

#include <tyr/common/variant.hpp>

namespace tyr
{

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::grammar::PredicateData<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::grammar::PredicateData<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Boolean<runir::kr::dl::NonemptyTag>>
{
    using Arg = ::cista::offset::variant<tyr::Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>,
                                         tyr::Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>>;

    Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::NonemptyTag>> index;
    Arg arg;

    Data() = default;
    explicit Data(Arg arg_) : index(), arg(std::move(arg_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

}  // namespace tyr

#endif
