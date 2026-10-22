#include "runir/kr/dl/translations/ygg_query.hpp"

namespace runir::kr::dl::detail
{

QueryTranslator::YggQuery QueryTranslator::translate(DlQuery query)
{
    if (const auto it = m_translated.find(query); it != m_translated.end())
        return it->second;
    const auto result = ygg::visit([&]<typename Tag>(QueryView<Tag> concrete) { return translate_concrete(query, concrete); }, query.get_variant());
    m_translated.emplace(query, result);
    return result;
}

QueryTranslator::YggQuery QueryTranslator::project(YggQuery query, std::span<const Label> labels)
{
    auto out = ygg::database::checkout<ygg::database::Query<ygg::database::QueryProjectTag>>(m_builder);
    out->arg = query.get_index();
    out->labels.set(labels.begin(), labels.end());
    return insert(*out);
}

std::vector<QueryTranslator::DlQuery> QueryTranslator::get_leaves(const ConstructorRepository& constructors) const
{
    auto leaves = std::vector<DlQuery>();
    for (const auto& slot : m_slots)
    {
        if (slot.kind != QuerySlot::Kind::QUERY)
            throw std::logic_error("Query translation: a slot is not a leaf query.");
        leaves.push_back(ygg::make_view(ygg::Index<Query<>>(slot.index), constructors));
    }
    return leaves;
}

QueryTranslator::YggQuery QueryTranslator::input(QuerySlot slot)
{
    auto out = ygg::database::checkout<ygg::database::Query<ygg::database::QueryInputTag>>(m_builder);
    out->input_slot = m_slots.size();
    out->columns.set(slot.columns.begin(), slot.columns.end());
    m_slots.push_back(std::move(slot));
    return insert(*out);
}

std::span<const ygg::database::ColumnLayout> QueryTranslator::schema(DlQuery query)
{
    return ygg::visit([](auto concrete) { return concrete.get_schema().span(); }, query.get_variant());
}

void QueryTranslator::flatten(DlQuery query, std::vector<DlQuery>& conjuncts)
{
    ygg::visit(
        [&]<typename Tag>(QueryView<Tag> concrete)
        {
            if constexpr (std::same_as<Tag, QueryJoinTag>)
            {
                flatten(concrete.get_lhs(), conjuncts);
                flatten(concrete.get_rhs(), conjuncts);
            }
            else
                conjuncts.push_back(query);
        },
        query.get_variant());
}

std::optional<QueryTranslator::YggQuery> QueryTranslator::rewrite_complements(DlQuery join, std::span<const Label> labels)
{
    auto conjuncts = std::vector<DlQuery>();
    flatten(join, conjuncts);
    auto positives = std::vector<DlQuery>();
    auto complements = std::vector<DlQuery>();
    for (const auto conjunct : conjuncts)
        (is_complement_leaf(conjunct) ? complements : positives).push_back(conjunct);
    const auto covered = [&](DlQuery complement)
    {
        return std::ranges::all_of(schema(complement),
                                   [&](const auto& column)
                                   {
                                       return std::ranges::any_of(positives,
                                                                  [&](DlQuery positive)
                                                                  {
                                                                      const auto columns = schema(positive);
                                                                      return std::ranges::find(columns, column.label, &ygg::database::ColumnLayout::label)
                                                                             != columns.end();
                                                                  });
                                   });
    };
    const auto uncovered = std::ranges::partition(complements, covered);
    positives.insert(positives.end(), uncovered.begin(), uncovered.end());
    complements.erase(uncovered.begin(), uncovered.end());
    if (complements.empty())
        return std::nullopt;
    auto result = translate(positives.front());
    for (size_t i = 1; i < positives.size(); ++i)
        result = binary<ygg::database::QueryJoinTag>(result, translate(positives[i]));
    for (const auto complement : complements)
        result = binary<ygg::database::QueryAntijoinTag>(result, complement_argument(complement));
    return project(result, labels);
}

QueryTranslator::YggQuery QueryTranslator::complement_argument(DlQuery complement)
{
    if (const auto it = m_arguments.find(complement); it != m_arguments.end())
        return it->second;
    auto slot = QuerySlot {};
    const auto columns = schema(complement);
    slot.columns.assign(columns.begin(), columns.end());
    slot.is_static = complement.is_static();
    visit_complement_argument(complement,
                              [&]<typename V>(V argument)
                              {
                                  constexpr auto role = std::same_as<V, ConstructorView<RoleTag>>;
                                  slot.kind = role ? QuerySlot::Kind::ROLE : QuerySlot::Kind::CONCEPT;
                                  slot.index = ygg::uint_t(argument.get_index());
                              });
    const auto result = input(std::move(slot));
    m_arguments.emplace(complement, result);
    return result;
}

template<typename Tag>
QueryTranslator::YggQuery QueryTranslator::translate_concrete(DlQuery query, QueryView<Tag> concrete)
{
    const auto& data = concrete.get_data();
    if constexpr (is_query_leaf_tag_v<Tag>)
    {
        const auto columns = concrete.get_schema().span();
        return input(QuerySlot { QuerySlot::Kind::QUERY, ygg::uint_t(query.get_index()), { columns.begin(), columns.end() }, query.is_static() });
    }
    else if constexpr (std::same_as<Tag, QueryJoinTag>)
    {
        if (m_rewrite_complements)
            if (const auto rewritten = rewrite_complements(query, query_labels(data.columns)))
                return *rewritten;
        return binary<ygg::database::QueryJoinTag>(translate(concrete.get_lhs()), translate(concrete.get_rhs()));
    }
    else if constexpr (std::same_as<Tag, QueryUnionTag>)
        return binary<ygg::database::QueryUnionTag>(translate(concrete.get_lhs()), translate(concrete.get_rhs()));
    else if constexpr (std::same_as<Tag, QueryDifferenceTag>)
        return binary<ygg::database::QueryDifferenceTag>(translate(concrete.get_lhs()), translate(concrete.get_rhs()));
    else if constexpr (std::same_as<Tag, QueryProjectTag>)
        return unary<ygg::database::QueryProjectTag>(translate(concrete.get_arg()), query_labels(data.columns));
    else if constexpr (std::same_as<Tag, QueryRenameTag>)
        return unary<ygg::database::QueryRenameTag>(translate(concrete.get_arg()), query_labels(data.columns));
    else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
    {
        auto out = ygg::database::checkout<ygg::database::Query<ygg::database::QuerySelectEqualTag>>(m_builder);
        out->arg = translate(concrete.get_arg()).get_index();
        out->lhs_column = Label(data.lhs_column.get_value());
        out->rhs_column = Label(data.rhs_column.get_value());
        return insert(*out);
    }
    else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
    {
        auto out = ygg::Data<ygg::database::Query<ygg::database::QuerySelectValueTag>>(translate(concrete.get_arg()).get_index(),
                                                                                       Label(data.column.get_value()),
                                                                                       data.object.get_value());
        return insert(out);
    }
    else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
    {
        auto out = ygg::database::checkout<ygg::database::Query<ygg::database::QueryGenericJoinTag>>(m_builder);
        for (const auto arg : concrete.get_args())
            out->inputs.push_back(translate(arg).get_index());
        const auto variable_order = query_labels(data.variable_order);
        const auto output_order = query_labels(data.output_order);
        out->variable_order.set(variable_order.begin(), variable_order.end());
        out->output_order.set(output_order.begin(), output_order.end());
        return insert(*out);
    }
    else if constexpr (std::same_as<Tag, QueryEmptyTag>)
    {
        auto out = ygg::database::checkout<ygg::database::Query<ygg::database::QueryEmptyTag>>(m_builder);
        const auto columns = concrete.get_schema().span();
        out->columns.set(columns.begin(), columns.end());
        return insert(*out);
    }
    else
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL query constructor");
}

}  // namespace runir::kr::dl::detail
