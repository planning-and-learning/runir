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
class View<Index<runir::kr::dl::QueryColumn>, C> : public ygg::IndexViewBase<runir::kr::dl::QueryColumn, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::QueryColumn, C>::IndexViewBase;

    const auto& get_name() const noexcept { return this->get_data().name; }

};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
class View<Index<runir::kr::dl::Query<Family, Tag>>, C> : public ygg::IndexViewBase<runir::kr::dl::Query<Family, Tag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::Query<Family, Tag>, C>::IndexViewBase;

    auto get_variant() const noexcept
        requires std::same_as<Tag, void>
    {
        return make_view(this->get_data().variant, this->get_context());
    }

    bool is_static() const noexcept
        requires std::same_as<Tag, void>
    {
        return this->get_data().is_static;
    }

    auto get_arg() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryConceptTag> || std::same_as<Tag, runir::kr::dl::QueryRoleTag>
                 || std::same_as<Tag, runir::kr::dl::QueryProjectTag> || std::same_as<Tag, runir::kr::dl::QueryRenameTag>
                 || std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag> || std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>)
    {
        return make_view(this->get_data().arg, this->get_context());
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    {
        return make_view(this->get_data().lhs, this->get_context());
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryUnionTag>
                 || std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
    {
        return make_view(this->get_data().rhs, this->get_context());
    }

    auto get_columns() const noexcept
    {
        if constexpr (std::same_as<Tag, void>)
            return ygg::visit([](auto concrete) { return concrete.get_columns(); }, get_variant());
        else
            return make_view(this->get_data().columns, this->get_context());
    }

    auto get_schema() const noexcept
    {
        if constexpr (std::same_as<Tag, void>)
            return ygg::visit([](auto concrete) { return concrete.get_schema(); }, get_variant());
        else if constexpr (std::same_as<Tag, runir::kr::dl::QueryJoinTag> || std::same_as<Tag, runir::kr::dl::QueryProjectTag>)
            return make_view(this->get_data().plan.output_columns().get_record(), this->get_context());
        else
            return make_view(this->get_data().schema.get_data(), this->get_context());
    }

    auto get_lhs_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    {
        return make_view(this->get_data().lhs_column, this->get_context());
    }

    auto get_rhs_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>
    {
        return make_view(this->get_data().rhs_column, this->get_context());
    }

    auto get_column() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    {
        return make_view(this->get_data().column, this->get_context());
    }

    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    {
        return make_view(this->get_data().predicate, this->get_context().get_planning_repository());
    }

    auto get_object() const noexcept
        requires std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>
    {
        return make_view(this->get_data().object, this->get_context().get_planning_repository());
    }

    auto get_polarity() const noexcept
        requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
    {
        return this->get_data().polarity;
    }

};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::ConceptOrRoleTag Category, typename C>
class View<Index<runir::kr::dl::QueryProjection<Family, Category>>, C> : public ygg::IndexViewBase<runir::kr::dl::QueryProjection<Family, Category>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::QueryProjection<Family, Category>, C>::IndexViewBase;

    auto get_arg() const noexcept { return make_view(this->get_data().arg, this->get_context()); }
    auto get_columns() const noexcept { return make_view(this->get_data().columns, this->get_context()); }

};

}  // namespace ygg

#endif
