#ifndef RUNIR_GRAMMAR_NUMERICAL_VIEW_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_VIEW_HPP_

#include "runir/kr/dl/grammar/numerical_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::grammar::Numerical<Tag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::grammar::Numerical<Tag>> m_handle;

public:
    View(Index<runir::kr::dl::grammar::Numerical<Tag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::kr::dl::CountTag>
    {
        return make_view(get_data().arg, *m_context);
    }

    auto get_lhs() const noexcept
        requires std::same_as<Tag, runir::kr::dl::DistanceTag>
    {
        return make_view(get_data().lhs, *m_context);
    }

    auto get_mid() const noexcept
        requires std::same_as<Tag, runir::kr::dl::DistanceTag>
    {
        return make_view(get_data().mid, *m_context);
    }

    auto get_rhs() const noexcept
        requires std::same_as<Tag, runir::kr::dl::DistanceTag>
    {
        return make_view(get_data().rhs, *m_context);
    }

    auto identifying_members() const noexcept { return std::tie(get_data()); }
};

}

#endif
