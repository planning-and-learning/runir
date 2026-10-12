#ifndef RUNIR_KR_DL_TRANSLATIONS_YGG_QUERY_HPP_
#define RUNIR_KR_DL_TRANSLATIONS_YGG_QUERY_HPP_

#include "runir/kr/dl/complement.hpp"
#include "runir/kr/dl/query_construction.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/distance/feature.hpp"

#include <algorithm>
#include <span>
#include <vector>
#include <yggdrasil/database/syntax/query.hpp>

namespace runir::kr::dl::detail
{

/// Lowers a DL edge query into a yggdrasil query whose input slots are its leaves. A complemented leaf joined with
/// conjuncts that bind its columns becomes an anti-join, A ⋈ ¬R = A \ (A ⋈ R), so the complement is never materialized.
template<FamilyTag Family, typename C>
class EdgeLowering
{
    using DlQuery = ygg::View<ygg::Index<Query<Family>>, C>;
    using YggQuery = ygg::database::QueryView<QueryValues>;

    ygg::database::QueryRepositoryFactory<QueryValues> m_factory;
    ygg::database::QueryRepository<QueryValues> m_repository;
    ygg::database::QueryBuilder<QueryValues> m_builder;
    std::vector<semantics::QuerySlot>& m_slots;

    template<typename Tag>
    YggQuery insert(ygg::Data<ygg::database::Query<QueryValues, Tag>>& data)
    {
        return ygg::database::insert_query(m_repository, m_builder, data);
    }
    template<typename Tag>
    YggQuery binary(YggQuery lhs, YggQuery rhs)
    {
        auto out = ygg::database::checkout<ygg::database::Query<QueryValues, Tag>>(m_builder);
        out->lhs = lhs.get_index();
        out->rhs = rhs.get_index();
        return insert(*out);
    }
    YggQuery project(YggQuery arg, std::span<const ygg::Index<ygg::database::Column>> labels)
    {
        auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QueryProjectTag>>(m_builder);
        out->arg = arg.get_index();
        out->labels.set(labels.begin(), labels.end());
        return insert(*out);
    }
    static std::span<const ygg::database::ColumnLayout> schema(DlQuery query)
    {
        return ygg::visit([](auto concrete) { return concrete.get_schema().span(); }, query.get_variant());
    }

public:
    explicit EdgeLowering(std::vector<semantics::QuerySlot>& slots) : m_repository(m_factory.create()), m_slots(slots) {}

    YggQuery input(std::span<const ygg::database::ColumnLayout> columns, semantics::QuerySlot slot)
    {
        auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QueryInputTag>>(m_builder);
        out->input_slot = m_slots.size();
        out->columns.set(columns.begin(), columns.end());
        m_slots.push_back(slot);
        return insert(*out);
    }

    YggQuery lower(DlQuery query)
    {
        return ygg::visit(
            [&]<typename Tag>(ygg::View<ygg::Index<Query<Family, Tag>>, C> concrete) -> YggQuery
            {
                const auto& data = concrete.get_data();
                if constexpr (std::same_as<Tag, QueryJoinTag>)
                {
                    const auto lhs = concrete.get_lhs();
                    const auto rhs = concrete.get_rhs();
                    const auto covered = [&](DlQuery side, DlQuery other)
                    {
                        return is_complement_leaf(side)
                               && std::ranges::all_of(schema(side),
                                                      [&](const auto& column)
                                                      { return std::ranges::find(schema(other), column.label, &ygg::database::ColumnLayout::label) != schema(other).end(); });
                    };
                    const auto anti_join = [&](DlQuery other, DlQuery complement)
                    {
                        const auto kept = lower(other);
                        const auto removed = input(schema(complement), semantics::QuerySlot { ygg::uint_t(complement.get_index()), true, complement.is_static() });
                        const auto labels = query_labels(data.columns);
                        return project(binary<ygg::database::QueryDifferenceTag>(kept, binary<ygg::database::QueryJoinTag>(kept, removed)), labels);
                    };
                    if (covered(rhs, lhs))
                        return anti_join(lhs, rhs);
                    if (covered(lhs, rhs))
                        return anti_join(rhs, lhs);
                    return binary<ygg::database::QueryJoinTag>(lower(lhs), lower(rhs));
                }
                else if constexpr (std::same_as<Tag, QueryUnionTag>)
                    return binary<ygg::database::QueryUnionTag>(lower(concrete.get_lhs()), lower(concrete.get_rhs()));
                else if constexpr (std::same_as<Tag, QueryDifferenceTag>)
                    return binary<ygg::database::QueryDifferenceTag>(lower(concrete.get_lhs()), lower(concrete.get_rhs()));
                else if constexpr (std::same_as<Tag, QueryProjectTag>)
                    return project(lower(concrete.get_arg()), query_labels(data.columns));
                else if constexpr (std::same_as<Tag, QueryRenameTag>)
                {
                    auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QueryRenameTag>>(m_builder);
                    out->arg = lower(concrete.get_arg()).get_index();
                    const auto labels = query_labels(data.columns);
                    out->labels.set(labels.begin(), labels.end());
                    return insert(*out);
                }
                else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
                {
                    auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QuerySelectEqualTag>>(m_builder);
                    out->arg = lower(concrete.get_arg()).get_index();
                    out->lhs_column = ygg::Index<ygg::database::Column>(data.lhs_column.get_value());
                    out->rhs_column = ygg::Index<ygg::database::Column>(data.rhs_column.get_value());
                    return insert(*out);
                }
                else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
                {
                    auto out = ygg::Data<ygg::database::Query<QueryValues, ygg::database::QuerySelectValueTag>>(
                        lower(concrete.get_arg()).get_index(), ygg::Index<ygg::database::Column>(data.column.get_value()), data.object);
                    return insert(out);
                }
                else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
                {
                    auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QueryGenericJoinTag>>(m_builder);
                    for (const auto arg : concrete.get_args())
                        out->inputs.push_back(lower(arg).get_index());
                    const auto variable_order = query_labels(data.variable_order);
                    const auto output_order = query_labels(data.output_order);
                    out->variable_order.set(variable_order.begin(), variable_order.end());
                    out->output_order.set(output_order.begin(), output_order.end());
                    return insert(*out);
                }
                else if constexpr (std::same_as<Tag, QueryEmptyTag>)
                {
                    auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QueryEmptyTag>>(m_builder);
                    const auto columns = concrete.get_schema().span();
                    out->columns.set(columns.begin(), columns.end());
                    return insert(*out);
                }
                else
                    // Atomic state and goal queries, concepts and roles.
                    return input(concrete.get_schema().span(), semantics::QuerySlot { ygg::uint_t(query.get_index()), false, query.is_static() });
            },
            query.get_variant());
    }
};

}  // namespace runir::kr::dl::detail

#endif
