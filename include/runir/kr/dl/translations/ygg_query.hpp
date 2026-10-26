#ifndef RUNIR_KR_DL_TRANSLATIONS_YGG_QUERY_HPP_
#define RUNIR_KR_DL_TRANSLATIONS_YGG_QUERY_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/query_construction.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/repository.hpp"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/core/concepts.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/database/query/query.hpp>
#include <yggdrasil/database/query/rewrite/relaxation.hpp>

namespace runir::kr::dl
{

/// An input slot of a translated query: the expression whose rows fill it, a query, or a concept or role lifted into
/// `columns` (the argument of a complemented leaf, or role edges), and whether its rows are the same in every state.
struct QuerySlot
{
    enum class Kind : std::uint8_t
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
template<ygg::InvocableWithEach<ygg::TypeList<ConstructorView<ConceptTag>, ConstructorView<RoleTag>, QueryView<>>> F>
auto visit_slot(const QuerySlot& slot, const ConstructorRepository& constructors, F f)
{
    switch (slot.kind)
    {
        case QuerySlot::Kind::CONCEPT:
            return f(ygg::make_view(ygg::Index<Constructor<ConceptTag>>(slot.index), constructors));
        case QuerySlot::Kind::ROLE:
            return f(ygg::make_view(ygg::Index<Constructor<RoleTag>>(slot.index), constructors));
        case QuerySlot::Kind::QUERY:
            break;
    }
    return f(ygg::make_view(ygg::Index<Query<>>(slot.index), constructors));
}

namespace detail
{

template<typename Tag>
inline constexpr bool is_query_leaf_tag_v = is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag> || std::same_as<Tag, QueryConceptTag>
                                            || std::same_as<Tag, QueryRoleTag> || std::same_as<Tag, QueryMustPassTag> || std::same_as<Tag, QueryHeadTag>;

template<typename V>
inline constexpr bool is_complement_view_v = false;
template<typename C>
inline constexpr bool is_complement_view_v<ygg::View<ygg::Index<Role<ComplementTag>>, C>> = true;
template<typename C>
inline constexpr bool is_complement_view_v<ygg::View<ygg::Index<Concept<NegationTag>>, C>> = true;

/// Calls f(argument) if the query is a role or concept leaf over a complement (r_complement, c_not); returns whether.
template<ygg::InvocableWithEach<ygg::TypeList<ConstructorView<RoleTag>, ConstructorView<ConceptTag>>> F>
bool visit_complement_argument(QueryView<> query, F f)
{
    return ygg::visit(
        [&]<typename Tag>(QueryView<Tag> concrete)
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
inline bool is_complement_leaf(QueryView<> query)
{
    return visit_complement_argument(query, [](auto) {});
}

/// Copies a DL query into a yggdrasil query repository; its leaves become input slots, each distinct leaf one slot.
///
/// With `rewrite_complements`, complemented leaves ¬R in a join whose other conjuncts cover R's columns become
/// antijoins: the join's conjuncts are flattened, P is the join of the others, and P ⋈ ¬R = P ▷ R, so the complement is
/// never materialized. The result is projected onto the DL join's column order. A complement not covered this way is
/// an ordinary leaf.
class QueryTranslator
{
    using DlQuery = QueryView<>;
    using YggQuery = ygg::database::QueryView<>;
    using Label = ygg::Index<ygg::database::Column>;

public:
    explicit QueryTranslator(bool rewrite_complements = false) : m_repository(m_factory.create()), m_rewrite_complements(rewrite_complements) {}
    QueryTranslator(const QueryTranslator&) = delete;
    QueryTranslator& operator=(const QueryTranslator&) = delete;
    QueryTranslator(QueryTranslator&&) = delete;  // Translated views borrow the owned repository.
    QueryTranslator& operator=(QueryTranslator&&) = delete;
    ~QueryTranslator() = default;

    YggQuery translate(DlQuery query);

    /// The projection of a translated query onto labels, so the optimizer can push a parent's projection down.
    YggQuery project(YggQuery query, std::span<const Label> labels);

    /// The relaxations of translated queries whose input slots are known only where `known(slot)` holds
    /// (ygg::database::relax); built in this translator's repository.
    template<std::predicate<size_t> Known>
    ygg::database::Relaxation relax(std::span<const YggQuery> roots, Known&& known)
    { return ygg::database::relax(roots, std::forward<Known>(known), m_repository, m_builder); }

    /// The expression of each input slot, by slot.
    const std::vector<QuerySlot>& get_slots() const noexcept { return m_slots; }
    /// The DL leaf of each input slot; without complement rewriting, every slot is a leaf query.
    std::vector<DlQuery> get_leaves(const ConstructorRepository& constructors) const;

private:
    template<typename Tag>
    YggQuery insert(ygg::Data<ygg::database::Query<Tag>>& data)
    { return ygg::database::insert_query(m_repository, m_builder, data); }
    template<typename Tag>
    YggQuery binary(YggQuery lhs, YggQuery rhs)
    {
        auto out = ygg::database::checkout<ygg::database::Query<Tag>>(m_builder);
        out->lhs = lhs.get_index();
        out->rhs = rhs.get_index();
        return insert(*out);
    }
    template<typename Tag>
    YggQuery unary(YggQuery arg, std::span<const Label> labels)
    {
        auto out = ygg::database::checkout<ygg::database::Query<Tag>>(m_builder);
        out->arg = arg.get_index();
        out->labels.set(labels.begin(), labels.end());
        return insert(*out);
    }
    YggQuery input(QuerySlot slot);
    static std::span<const ygg::database::ColumnLayout> schema(DlQuery query);

    /// The conjuncts of a tree of binary joins.
    static void flatten(DlQuery query, std::vector<DlQuery>& conjuncts);

    /// The complement rewrite of a join (see the class comment), or nothing if no complement is covered.
    std::optional<YggQuery> rewrite_complements(DlQuery join, std::span<const Label> labels);

    /// The input slot of a complemented leaf's argument, lifted into the leaf's columns.
    YggQuery complement_argument(DlQuery complement);

    template<typename Tag>
    YggQuery translate_concrete(DlQuery query, QueryView<Tag> concrete);

    ygg::database::QueryRepositoryFactory m_factory;
    ygg::database::QueryRepository m_repository;
    ygg::database::QueryBuilder m_builder;
    bool m_rewrite_complements;
    ygg::UnorderedMap<DlQuery, YggQuery> m_translated;
    ygg::UnorderedMap<DlQuery, YggQuery> m_arguments;  ///< The slot of each complemented leaf's argument.
    std::vector<QuerySlot> m_slots;
};

}  // namespace detail
}  // namespace runir::kr::dl

#endif
