#ifndef RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/grammar/boolean_data.hpp"
#include <yggdrasil/containers/variant.hpp>

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::dl::grammar::Numerical<runir::kr::dl::CountTag>>
{
    using ConstructorVariant = ::ygg::IndexVariant<GrammarArgumentTypes>;
    using Arg = ConstructorVariant;
    Index<runir::kr::dl::grammar::Numerical<runir::kr::dl::CountTag>> index;
    ConstructorVariant arg;
    Data() = default;
    explicit Data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<ConstructorVariant, C>;
    template<typename C>
    explicit Data(const ViewVariant<C>& arg_) :
        index(),
        arg(std::visit([](const auto& view) -> ConstructorVariant { return ConstructorVariant(view.get_index()); }, arg_))
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
struct Data<runir::kr::dl::grammar::Numerical<runir::kr::dl::DistanceTag>> :
    runir::kr::dl::TernaryData<runir::kr::dl::grammar::Numerical<runir::kr::dl::DistanceTag>, GrammarConceptChoice, GrammarRoleChoice, GrammarConceptChoice>
{
    using Base = runir::kr::dl::
        TernaryData<runir::kr::dl::grammar::Numerical<runir::kr::dl::DistanceTag>, GrammarConceptChoice, GrammarRoleChoice, GrammarConceptChoice>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Numerical<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> :
    runir::kr::dl::ArgumentData<runir::kr::dl::grammar::Numerical<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::ArgumentData<runir::kr::dl::grammar::Numerical<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                             runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Numerical<runir::kr::dl::NumericalConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::grammar::Numerical<runir::kr::dl::NumericalConstantTag>, ygg::uint_t>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::grammar::Numerical<runir::kr::dl::NumericalConstantTag>, ygg::uint_t>;
    using Base::Base;
};

template<runir::kr::dl::NumericalBinaryTag Tag>
struct Data<runir::kr::dl::grammar::Numerical<Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::grammar::Numerical<Tag>,
                              runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>,
                              runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Numerical<Tag>,
                                           runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>,
                                           runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
