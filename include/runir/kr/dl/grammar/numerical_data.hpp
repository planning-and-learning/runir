#ifndef RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/grammar/boolean_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <utility>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::CountTag>>
{
    using ConstructorVariant = ::cista::offset::variant<Index<GrammarConceptChoice<Family>>, Index<GrammarRoleChoice<Family>>>;
    using Arg = ConstructorVariant;
    Index<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::CountTag>> index;
    ConstructorVariant arg;
    Data() = default;
    explicit Data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}
    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(arg);
    }
    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::DistanceTag>> :
    runir::kr::dl::semantics::TernaryData<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::DistanceTag>,
                                          GrammarConceptChoice<Family>,
                                          GrammarRoleChoice<Family>,
                                          GrammarConceptChoice<Family>>
{
    using Base = runir::kr::dl::semantics::TernaryData<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::DistanceTag>,
                                                       GrammarConceptChoice<Family>,
                                                       GrammarRoleChoice<Family>,
                                                       GrammarConceptChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> :
    runir::kr::dl::semantics::ArgumentData<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                           runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::semantics::ArgumentData<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                                        runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
