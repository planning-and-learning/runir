#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/numerical_data.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag, formalism::SymbolContextFor<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>> C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>>, C> :
    public formalism::detail::View<Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>>, C>
{
public:
    View(Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>>, C>(handle, context)
    {
    }

    auto get_identifier() const noexcept
        requires std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>
    {
        return this->get_data().identifier;
    }

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::kr::dl::CountTag>
    {
        return make_view(this->get_data().arg, *this->m_context);
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::DistanceTag> || runir::kr::dl::NumericalBinaryTag<Tag>)
    {
        return make_view(this->get_data().lhs, *this->m_context);
    }

    auto get_mid() const noexcept
        requires std::same_as<Tag, runir::kr::dl::DistanceTag>
    {
        return make_view(this->get_data().mid, *this->m_context);
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::DistanceTag> || runir::kr::dl::NumericalBinaryTag<Tag>)
    {
        return make_view(this->get_data().rhs, *this->m_context);
    }

    auto get_value() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NumericalConstantTag>
    {
        return this->get_data().identifier;
    }
};

}

#endif
