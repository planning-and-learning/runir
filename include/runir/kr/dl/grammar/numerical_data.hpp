#ifndef RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/grammar/boolean_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <utility>

namespace ygg
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
        ygg::clear(index);
        ygg::clear(arg);
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

}  // namespace ygg

#endif
