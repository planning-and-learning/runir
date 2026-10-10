#ifndef RUNIR_GRAMMAR_NUMERICAL_VIEW_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_VIEW_HPP_

#include "runir/kr/dl/grammar/numerical_data.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag, formalism::SymbolContextFor<runir::kr::dl::grammar::Numerical<Family, Tag>> C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::grammar::Numerical<Family, Tag>>, C> : public ygg::IndexViewBase<runir::kr::dl::grammar::Numerical<Family, Tag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::grammar::Numerical<Family, Tag>, C>::IndexViewBase;

    auto get_identifier() const noexcept
        requires std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>
    {
        return this->get_data().identifier;
    }

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::kr::dl::CountTag>
    {
        return make_view(this->get_data().arg, this->get_context());
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::DistanceTag> || runir::kr::dl::NumericalBinaryTag<Tag>)
    {
        return make_view(this->get_data().lhs, this->get_context());
    }

    auto get_mid() const noexcept
        requires std::same_as<Tag, runir::kr::dl::DistanceTag>
    {
        return make_view(this->get_data().mid, this->get_context());
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::DistanceTag> || runir::kr::dl::NumericalBinaryTag<Tag>)
    {
        return make_view(this->get_data().rhs, this->get_context());
    }

    auto get_value() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NumericalConstantTag>
    {
        return this->get_data().identifier;
    }
};

}

#endif
