#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/data_helpers.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::CountTag>>
{
    using Arg = ::cista::offset::variant<ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>,
                                         ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>>;

    Index<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::CountTag>> index;
    Arg arg;

    Data() = default;
    explicit Data(Arg arg_) : index(), arg(std::move(arg_)) {}

    auto cista_members() noexcept { return std::tie(index, arg); }
    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::DistanceTag>> :
    runir::kr::dl::TernaryData<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::DistanceTag>,
                               runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>,
                               runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                               runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::TernaryData<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::DistanceTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> :
    runir::kr::dl::ArgumentData<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>
{
    using Base =
        runir::kr::dl::ArgumentData<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                    runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::NumericalConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalBinaryTag Tag>
struct Data<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>,
                              runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>,
                              runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
