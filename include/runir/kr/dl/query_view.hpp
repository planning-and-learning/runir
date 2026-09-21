#ifndef RUNIR_KR_DL_QUERY_VIEW_HPP_
#define RUNIR_KR_DL_QUERY_VIEW_HPP_

#include "runir/kr/dl/query_data.hpp"

#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::dl::QueryColumn>, C>
{
    Index<runir::kr::dl::QueryColumn> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::dl::QueryColumn> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    const auto& get_name() const noexcept { return get_data().name; }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
class View<Index<runir::kr::dl::Query<Family, Tag>>, C>
{
    Index<runir::kr::dl::Query<Family, Tag>> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::dl::Query<Family, Tag>> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto get_variant() const noexcept
        requires std::same_as<Tag, void>
    {
        return make_view(get_data().variant, *m_context);
    }

    auto get_arg() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryConceptTag> || std::same_as<Tag, runir::kr::dl::QueryRoleTag>
                 || std::same_as<Tag, runir::kr::dl::QueryProjectTag> || std::same_as<Tag, runir::kr::dl::QueryRenameTag>
                 || std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag> || std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>)
    {
        return make_view(get_data().arg, *m_context);
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    {
        return make_view(get_data().lhs, *m_context);
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    {
        return make_view(get_data().rhs, *m_context);
    }

    auto get_columns() const noexcept
    {
        if constexpr (std::same_as<Tag, void>)
            return ygg::visit([](auto concrete) { return concrete.get_columns(); }, get_variant());
        else
            return make_view(get_data().columns, *m_context);
    }

    auto get_schema() const noexcept
    {
        if constexpr (std::same_as<Tag, void>)
            return ygg::visit([](auto concrete) { return concrete.get_schema(); }, get_variant());
        else if constexpr (std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryProjectTag>)
            return get_data().plan.output_columns();
        else
            return get_data().schema.view();
    }

    auto get_lhs_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    {
        return make_view(get_data().lhs_column, *m_context);
    }

    auto get_rhs_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    {
        return make_view(get_data().rhs_column, *m_context);
    }

    auto get_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    {
        return make_view(get_data().column, *m_context);
    }

    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    {
        return make_view(get_data().predicate, m_context->get_planning_repository());
    }

    auto get_object() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    {
        return make_view(get_data().object, m_context->get_planning_repository());
    }

    auto get_polarity() const noexcept
        requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
    {
        return get_data().polarity;
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::QueryProjection<Family, Category>>, C>
{
    static_assert(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>);

private:
    Index<runir::kr::dl::QueryProjection<Family, Category>> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::dl::QueryProjection<Family, Category>> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_arg() const noexcept { return make_view(get_data().arg, *m_context); }
    auto get_columns() const noexcept { return make_view(get_data().columns, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
