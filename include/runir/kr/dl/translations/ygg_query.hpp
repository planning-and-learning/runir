#ifndef RUNIR_KR_DL_TRANSLATIONS_YGG_QUERY_HPP_
#define RUNIR_KR_DL_TRANSLATIONS_YGG_QUERY_HPP_

#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/query_construction.hpp"
#include "runir/kr/dl/query_view.hpp"

#include <algorithm>
#include <concepts>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/database/syntax/query.hpp>
#include <yggdrasil/database/syntax/translations/relaxation.hpp>

namespace runir::kr::dl
{

/// An input slot of a translated query: the expression whose rows fill it, a query, or a concept or role lifted into
/// `columns` (the argument of a complemented leaf, or role edges), and whether its rows are the same in every state.
struct QuerySlot
{
    enum class Kind
    {
        QUERY,
        CONCEPT,
        ROLE
    };
    Kind kind = Kind::QUERY;
    ygg::uint_t index = 0;
    std::vector<ygg::database::ColumnLayout> columns;
    bool is_static = false;
};

/// Calls f(expression) with the slot's expression as a view into `constructors`.
template<FamilyTag Family, typename C, typename F>
decltype(auto) visit_slot(const QuerySlot& slot, const C& constructors, F&& f)
{
    switch (slot.kind)
    {
        case QuerySlot::Kind::CONCEPT:
            return f(ygg::make_view(ygg::Index<Constructor<Family, ConceptTag>>(slot.index), constructors));
        case QuerySlot::Kind::ROLE:
            return f(ygg::make_view(ygg::Index<Constructor<Family, RoleTag>>(slot.index), constructors));
        case QuerySlot::Kind::QUERY:
            break;
    }
    return f(ygg::make_view(ygg::Index<Query<Family>>(slot.index), constructors));
}

namespace detail
{

template<typename Tag>
inline constexpr bool is_query_leaf_tag_v =
    is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag> || std::same_as<Tag, QueryConceptTag> || std::same_as<Tag, QueryRoleTag>;

template<typename V>
inline constexpr bool is_complement_view_v = false;
template<FamilyTag Family, typename C>
inline constexpr bool is_complement_view_v<ygg::View<ygg::Index<FamilyRole<Family, ComplementTag>>, C>> = true;
template<FamilyTag Family, typename C>
inline constexpr bool is_complement_view_v<ygg::View<ygg::Index<FamilyConcept<Family, NegationTag>>, C>> = true;

/// Calls f(argument) if the query is a role or concept leaf over a complement (r_complement, c_not); returns whether.
template<FamilyTag Family, typename C, typename F>
bool visit_complement_argument(ygg::View<ygg::Index<Query<Family>>, C> query, F&& f)
{
    return ygg::visit(
        [&]<typename Tag>(ygg::View<ygg::Index<Query<Family, Tag>>, C> concrete)
        {
            if constexpr (std::same_as<Tag, QueryRoleTag> || std::same_as<Tag, QueryConceptTag>)
                return ygg::visit(
                    [&]<typename V>(V constructor)
                    {
                        if constexpr (is_complement_view_v<V>)
                        {
                            f(constructor.get_arg());
                            return true;
                        }
                        else
                            return false;
                    },
                    concrete.get_arg().get_variant());
            else
                return false;
        },
        query.get_variant());
}

/// Whether a query is a role or concept leaf over a complement (r_complement, c_not).
template<FamilyTag Family, typename C>
bool is_complement_leaf(ygg::View<ygg::Index<Query<Family>>, C> query)
{
    return visit_complement_argument(query, [](auto) {});
}

/// Copies a DL query into a yggdrasil query repository; its leaves become input slots, each distinct leaf one slot.
///
/// With `rewrite_complements`, complemented leaves ¬R in a join whose other conjuncts cover R's columns become
/// antijoins: the join's conjuncts are flattened, P is the join of the others, and P ⋈ ¬R = P ▷ R, so the complement is
/// never materialized. The result is projected onto the DL join's column order. A complement not covered this way is
/// an ordinary leaf.
template<FamilyTag Family, typename C = ConstructorRepositoryFor<Family>>
class QueryTranslator
{
    using DlQuery = ygg::View<ygg::Index<Query<Family>>, C>;
    using YggQuery = ygg::database::QueryView<QueryValues>;
    using Label = ygg::Index<ygg::database::Column>;

public:
    explicit QueryTranslator(bool rewrite_complements = false) : m_repository(m_factory.create()), m_rewrite_complements(rewrite_complements) {}
    QueryTranslator(const QueryTranslator&) = delete;
    QueryTranslator& operator=(const QueryTranslator&) = delete;

    YggQuery translate(DlQuery query)
    {
        if (const auto it = m_translated.find(query); it != m_translated.end())
            return it->second;
        const auto result = ygg::visit([&]<typename Tag>(ygg::View<ygg::Index<Query<Family, Tag>>, C> concrete) { return translate_concrete(query, concrete); },
                                       query.get_variant());
        m_translated.emplace(query, result);
        return result;
    }

    /// The projection of a translated query onto labels, so the optimizer can push a parent's projection down.
    YggQuery project(YggQuery query, std::span<const Label> labels)
    {
        auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QueryProjectTag>>(m_builder);
        out->arg = query.get_index();
        out->labels.set(labels.begin(), labels.end());
        return insert(*out);
    }

    /// The relaxations of translated queries whose input slots are known only where `known(slot)` holds
    /// (ygg::database::relax); built in this translator's repository.
    template<std::predicate<size_t> Known>
    ygg::database::Relaxation<QueryValues> relax(std::span<const YggQuery> roots, Known&& known)
    {
        return ygg::database::relax(roots, std::forward<Known>(known), m_repository, m_builder);
    }

    /// The expression of each input slot, by slot.
    const std::vector<QuerySlot>& get_slots() const noexcept { return m_slots; }
    /// The DL leaf of each input slot; without complement rewriting, every slot is a leaf query.
    std::vector<DlQuery> get_leaves(const C& constructors) const
    {
        auto leaves = std::vector<DlQuery>();
        for (const auto& slot : m_slots)
        {
            if (slot.kind != QuerySlot::Kind::QUERY)
                throw std::logic_error("Query translation: a slot is not a leaf query.");
            leaves.push_back(ygg::make_view(ygg::Index<Query<Family>>(slot.index), constructors));
        }
        return leaves;
    }

private:
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
    template<typename Tag>
    YggQuery unary(YggQuery arg, std::span<const Label> labels)
    {
        auto out = ygg::database::checkout<ygg::database::Query<QueryValues, Tag>>(m_builder);
        out->arg = arg.get_index();
        out->labels.set(labels.begin(), labels.end());
        return insert(*out);
    }
    YggQuery input(QuerySlot slot)
    {
        auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QueryInputTag>>(m_builder);
        out->input_slot = m_slots.size();
        out->columns.set(slot.columns.begin(), slot.columns.end());
        m_slots.push_back(std::move(slot));
        return insert(*out);
    }
    static std::span<const ygg::database::ColumnLayout> schema(DlQuery query)
    {
        return ygg::visit([](auto concrete) { return concrete.get_schema().span(); }, query.get_variant());
    }

    /// The conjuncts of a tree of binary joins.
    static void flatten(DlQuery query, std::vector<DlQuery>& conjuncts)
    {
        ygg::visit(
            [&]<typename Tag>(ygg::View<ygg::Index<Query<Family, Tag>>, C> concrete)
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

    /// The complement rewrite of a join (see the class comment), or nothing if no complement is covered.
    std::optional<YggQuery> rewrite_complements(DlQuery join, std::span<const Label> labels)
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

    /// The input slot of a complemented leaf's argument, lifted into the leaf's columns.
    YggQuery complement_argument(DlQuery complement)
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
                                      constexpr auto role = std::same_as<V, ygg::View<ygg::Index<Constructor<Family, RoleTag>>, C>>;
                                      slot.kind = role ? QuerySlot::Kind::ROLE : QuerySlot::Kind::CONCEPT;
                                      slot.index = ygg::uint_t(argument.get_index());
                                  });
        const auto result = input(std::move(slot));
        m_arguments.emplace(complement, result);
        return result;
    }

    template<typename Tag>
    YggQuery translate_concrete(DlQuery query, ygg::View<ygg::Index<Query<Family, Tag>>, C> concrete)
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
            auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QuerySelectEqualTag>>(m_builder);
            out->arg = translate(concrete.get_arg()).get_index();
            out->lhs_column = Label(data.lhs_column.get_value());
            out->rhs_column = Label(data.rhs_column.get_value());
            return insert(*out);
        }
        else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
        {
            auto out = ygg::Data<ygg::database::Query<QueryValues, ygg::database::QuerySelectValueTag>>(translate(concrete.get_arg()).get_index(),
                                                                                                       Label(data.column.get_value()),
                                                                                                       data.object);
            return insert(out);
        }
        else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
        {
            auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QueryGenericJoinTag>>(m_builder);
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
            auto out = ygg::database::checkout<ygg::database::Query<QueryValues, ygg::database::QueryEmptyTag>>(m_builder);
            const auto columns = concrete.get_schema().span();
            out->columns.set(columns.begin(), columns.end());
            return insert(*out);
        }
        else
            static_assert(ygg::dependent_false<Tag>::value, "unhandled DL query constructor");
    }

    ygg::database::QueryRepositoryFactory<QueryValues> m_factory;
    ygg::database::QueryRepository<QueryValues> m_repository;
    ygg::database::QueryBuilder<QueryValues> m_builder;
    bool m_rewrite_complements;
    ygg::UnorderedMap<DlQuery, YggQuery> m_translated;
    ygg::UnorderedMap<DlQuery, YggQuery> m_arguments;  ///< The slot of each complemented leaf's argument.
    std::vector<QuerySlot> m_slots;
};

}  // namespace detail
}  // namespace runir::kr::dl

#endif
