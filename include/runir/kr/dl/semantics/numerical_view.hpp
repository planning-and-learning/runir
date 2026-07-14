#ifndef RUNIR_SEMANTICS_NUMERICAL_VIEW_HPP_
#define RUNIR_SEMANTICS_NUMERICAL_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/numerical_data.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::FamilyNumerical<Family, Tag>> m_handle;

public:
    View(Index<runir::kr::dl::FamilyNumerical<Family, Tag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto get_argument() const noexcept
        requires std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>
    {
        return make_view(get_data().reference, *m_context);
    }

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::kr::dl::CountTag>
    {
        return make_view(get_data().arg, *m_context);
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::DistanceTag> || runir::kr::dl::NumericalBinaryTag<Tag>)
    {
        return make_view(get_data().lhs, *m_context);
    }

    auto get_mid() const noexcept
        requires std::same_as<Tag, runir::kr::dl::DistanceTag>
    {
        return make_view(get_data().mid, *m_context);
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::DistanceTag> || runir::kr::dl::NumericalBinaryTag<Tag>)
    {
        return make_view(get_data().rhs, *m_context);
    }

    auto get_value() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NumericalConstantTag>
    {
        return get_data().identifier;
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
