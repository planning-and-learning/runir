#ifndef RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/grammar/data_helpers.hpp"

#include <tyr/common/variant.hpp>

namespace tyr
{

template<>
struct Data<runir::grammar::Numerical<runir::CountTag>>
{
    using Arg = ::cista::offset::variant<tyr::Index<runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>,
                                         tyr::Index<runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>>;

    Index<runir::grammar::Numerical<runir::CountTag>> index;
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

template<>
struct Data<runir::grammar::Numerical<runir::DistanceTag>> :
    runir::grammar::TernaryData<runir::grammar::Numerical<runir::DistanceTag>,
                                runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>,
                                runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                                runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>
{
    using Base = runir::grammar::TernaryData<runir::grammar::Numerical<runir::DistanceTag>,
                                             runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>,
                                             runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                                             runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
