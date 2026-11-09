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
#include <urd/repr/core/dependent_false.hpp>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <boost/mp11.hpp>
#include <tyr/datalog/lifted/database/syntax/query.hpp>
#include <tyr/datalog/lifted/database/syntax/translations/relaxation.hpp>

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
    urd::Store<urd::sequence<ygg::ColumnLayout>> columns;
    bool is_static = false;
};

/// Calls f(expression) with the slot's expression as a view into `constructors`.
template<typename F>
    requires boost::mp11::mp_all_of_q<boost::mp11::mp_list<ConstructorView<ConceptTag>, ConstructorView<RoleTag>, QueryView<>>,
                                      boost::mp11::mp_bind_front<std::is_invocable, F>>::value
auto visit_slot(const QuerySlot& slot, const ConstructorRepository& constructors, F f)
{
    switch (slot.kind)
    {
        case QuerySlot::Kind::CONCEPT:
            return f(constructors.view(urd::index<Constructor<ConceptTag>>(slot.index)));
        case QuerySlot::Kind::ROLE:
            return f(constructors.view(urd::index<Constructor<RoleTag>>(slot.index)));
        case QuerySlot::Kind::QUERY:
            break;
    }
    return f(constructors.view(urd::index<Query<>>(slot.index)));
}

namespace detail
{

template<typename Tag>
inline constexpr bool is_query_leaf_tag_v = is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag> || std::same_as<Tag, QueryConceptTag>
                                            || std::same_as<Tag, QueryRoleTag> || std::same_as<Tag, QueryMustPassTag>;

template<typename V>
inline constexpr bool is_complement_view_v = false;
template<typename C>
inline constexpr bool is_complement_view_v<urd::index_view<Role<SemanticFamily, ComplementTag>, C>> = true;
template<typename C>
inline constexpr bool is_complement_view_v<urd::index_view<Concept<SemanticFamily, NegationTag>, C>> = true;

/// Calls f(argument) if the query is a role or concept leaf over a complement (r_complement, c_not); returns whether.
template<typename F>
    requires boost::mp11::mp_all_of_q<boost::mp11::mp_list<ConstructorView<RoleTag>, ConstructorView<ConceptTag>>, boost::mp11::mp_bind_front<std::is_invocable, F>>::value
bool visit_complement_argument(QueryView<> query, F f)
{
    return std::visit(
        [&]<typename Tag>(QueryView<Tag> concrete)
        {
            if constexpr (std::same_as<Tag, QueryRoleTag> || std::same_as<Tag, QueryConceptTag>)
                return std::visit(
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
    using YggQuery = tyr::datalog::database::QueryView<>;
    using Label = urd::index<ygg::Column>;

public:
    explicit QueryTranslator(bool rewrite_complements = false) : m_rewrite_complements(rewrite_complements) {}
    QueryTranslator(const QueryTranslator&) = delete;
    QueryTranslator& operator=(const QueryTranslator&) = delete;
    QueryTranslator(QueryTranslator&&) = delete;  // Translated views borrow the owned repository.
    QueryTranslator& operator=(QueryTranslator&&) = delete;
    ~QueryTranslator() = default;

    YggQuery translate(DlQuery query);

    /// The projection of a translated query onto labels, so the optimizer can push a parent's projection down.
    YggQuery project(YggQuery query, std::span<const Label> labels);

    /// The relaxations of translated queries whose input slots are known only where `known(slot)` holds
    /// (tyr::datalog::database::relax); built in this translator's repository.
    template<std::predicate<size_t> Known>
    tyr::datalog::database::Relaxation relax(std::span<const YggQuery> roots, Known&& known)
    { return tyr::datalog::database::relax(roots, std::forward<Known>(known), m_repository, m_builder); }

    /// The expression of each input slot, by slot.
    const urd::Store<urd::sequence<QuerySlot>>& get_slots() const noexcept { return m_slots; }
    /// The DL leaf of each input slot; without complement rewriting, every slot is a leaf query.
    urd::Store<urd::sequence<DlQuery>> get_leaves(const ConstructorRepository& constructors) const;

private:
    template<typename Tag>
    YggQuery insert(urd::data<tyr::datalog::database::Query<Tag>>& data)
    { return tyr::datalog::database::insert_query(m_repository, m_builder, data); }
    template<typename Tag>
    YggQuery binary(YggQuery lhs, YggQuery rhs)
    {
        auto out = tyr::datalog::database::checkout<tyr::datalog::database::Query<Tag>>(m_builder);
        out->lhs = lhs.index();
        out->rhs = rhs.index();
        return insert(*out);
    }
    template<typename Tag>
    YggQuery unary(YggQuery arg, std::span<const Label> labels)
    {
        auto out = tyr::datalog::database::checkout<tyr::datalog::database::Query<Tag>>(m_builder);
        out->arg = arg.index();
        out->labels.set(labels.begin(), labels.end());
        return insert(*out);
    }
    YggQuery input(QuerySlot slot);
    static std::span<const ygg::ColumnLayout> schema(DlQuery query);

    /// The conjuncts of a tree of binary joins.
    static void flatten(DlQuery query, urd::Store<urd::sequence<DlQuery>>& conjuncts);

    /// The complement rewrite of a join (see the class comment), or nothing if no complement is covered.
    std::optional<YggQuery> rewrite_complements(DlQuery join, std::span<const Label> labels);

    /// The input slot of a complemented leaf's argument, lifted into the leaf's columns.
    YggQuery complement_argument(DlQuery complement);

    template<typename Tag>
    YggQuery translate_concrete(DlQuery query, QueryView<Tag> concrete);

    tyr::datalog::database::QueryRepository m_repository;
    tyr::datalog::database::QueryBuilder m_builder;
    bool m_rewrite_complements;
    urd::Store<urd::map<DlQuery, YggQuery>> m_translated;
    urd::Store<urd::map<DlQuery, YggQuery>> m_arguments;  ///< The slot of each complemented leaf's argument.
    urd::Store<urd::sequence<QuerySlot>> m_slots;
};

}  // namespace detail
}  // namespace runir::kr::dl

#endif
