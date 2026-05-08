#ifndef RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/grammar/data_helpers.hpp"

#include <tyr/common/variant.hpp>

namespace tyr
{

template<tyr::formalism::FactKind T>
struct Data<runir::grammar::Boolean<runir::AtomicStateTag<T>>> : runir::grammar::PredicateData<runir::grammar::Boolean<runir::AtomicStateTag<T>>, T>
{
    using Base = runir::grammar::PredicateData<runir::grammar::Boolean<runir::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Boolean<runir::NonemptyTag>>
{
    using Arg = ::cista::offset::variant<tyr::Index<runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>,
                                         tyr::Index<runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>>;

    Index<runir::grammar::Boolean<runir::NonemptyTag>> index;
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
