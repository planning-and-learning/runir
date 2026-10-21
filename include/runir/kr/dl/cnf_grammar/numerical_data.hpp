#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_

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

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::CountTag>>
{
    using Arg = ::ygg::IndexVariant<::ygg::MapTypeListSecondT<runir::kr::dl::cnf_grammar::NonTerminal, Family, runir::kr::dl::ConceptOrRoleTags>>;

    Index<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::CountTag>> index;
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
