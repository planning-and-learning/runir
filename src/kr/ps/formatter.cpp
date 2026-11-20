#include "runir/kr/ps/formatter.hpp"

#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/icp/repository.hpp"
#include "runir/kr/ps/proof_status.hpp"
#include "runir/kr/uns/repository.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <optional>
#include <string>
#include <urd/format.hpp>
#include <variant>

namespace rk = runir::kr;
namespace rp = runir::kr::ps;

namespace
{
/// The head of a q_head with `arity` columns; none for any other query.
template<typename C>
std::optional<std::string> query_head(urd::index_view<runir::kr::dl::Query<runir::kr::dl::QueryHeadTag>, C> view, size_t arity)
{
    if (view.get_columns().size() != arity)
        return std::nullopt;
    return std::string(view.get_relation().get_name());
}

template<typename V>
std::optional<std::string> query_head(V, size_t)
{ return std::nullopt; }

template<typename C>
std::optional<std::string> query_head(urd::index_view<runir::kr::dl::Query<>, C> view, size_t arity)
{
    return std::visit([&](auto concrete) { return query_head(concrete, arity); }, view.get_variant());
}

/// The head that a feature's expression reads whole, as `(:head h)` parses into (parse_head_feature in
/// src/kr/ps/ext/dl/parser.cpp): a projection of a q_head onto its columns in their order, b_nonempty of a q_head
/// without columns, or n_value of one with one column.
template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
std::optional<std::string> read_head(urd::index_view<runir::kr::dl::QueryProjection<Category>, C> view)
{
    const auto columns = view.get_columns();
    const auto arity = std::same_as<Category, runir::kr::dl::ConceptTag> ? size_t { 1 } : size_t { 2 };
    auto result = query_head(view.get_operand(), arity);
    if (result)
        std::visit(
            [&](auto concrete)
            {
                if constexpr (requires { concrete.get_relation(); })
                    if (!std::ranges::equal(concrete.get_columns(), columns))
                        result.reset();
            },
            view.get_operand().get_variant());
    return result;
}

template<typename C>
std::optional<std::string> read_head(urd::index_view<runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, runir::kr::dl::NonemptyTag>, C> view)
{
    return std::visit([&](auto operand) { return query_head(operand, 0); }, view.get_operand());
}

template<typename C>
std::optional<std::string> read_head(urd::index_view<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::ValueTag>, C> view)
{ return query_head(view.get_operand(), 1); }

template<typename V>
std::optional<std::string> read_head(V)
{ return std::nullopt; }
}  // namespace

template<rk::FamilyTag Family, rp::dl::FeatureTag FeatureTag, typename C>
fmt::format_context::iterator
fmt::formatter<urd::index_view<rp::Feature<Family, FeatureTag>, C>>::format(const urd::index_view<rp::Feature<Family, FeatureTag>, C>& value,
                                                                            fmt::format_context& context) const
{
    return std::visit([&](auto concrete) { return fmt::format_to(context.out(), "{}", concrete); }, value.get_variant());
}

template<rk::FamilyTag Family, rp::dl::FeatureTag FeatureTag, typename C>
fmt::format_context::iterator fmt::formatter<urd::index_view<rp::ConcreteFeature<Family, rk::DlTag, FeatureTag>, C>>::format(
    const urd::index_view<rp::ConcreteFeature<Family, rk::DlTag, FeatureTag>, C>& value,
    fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:{}", FeatureTag::keyword);
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:symbol {})", urd::new_line(), value.get_symbol());
        // A feature that reads a :datalog head whole prints as it is written, so that the program parses again.
        auto head = std::optional<std::string>();
        if constexpr (requires { value.get_feature().get_variant(); })
            head = std::visit([](auto concrete) { return read_head(concrete); }, value.get_feature().get_variant());
        if (head)
            out = fmt::format_to(out, "{}(:head {})", urd::new_line(), *head);
        else
            out = fmt::format_to(out, "{}(:expression {})", urd::new_line(), value.get_feature());
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

template<rk::FamilyTag Family, rp::dl::FeatureTag FeatureTag, rp::dl::ConditionObservationTag<FeatureTag> ObservationTag, typename C>
fmt::format_context::iterator fmt::formatter<urd::index_view<rp::ConcreteCondition<Family, rk::DlTag, FeatureTag, ObservationTag>, C>>::format(
    const urd::index_view<rp::ConcreteCondition<Family, rk::DlTag, FeatureTag, ObservationTag>, C>& value,
    fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "({} {})",
                          ObservationTag::keyword,
                          std::visit([](auto concrete) { return concrete.get_symbol(); }, value.get_feature().get_variant()));
}

template<rk::FamilyTag Family, rp::dl::FeatureTag FeatureTag, rp::dl::EffectObservationTag<FeatureTag> ObservationTag, typename C>
fmt::format_context::iterator fmt::formatter<urd::index_view<rp::ConcreteEffect<Family, rk::DlTag, FeatureTag, ObservationTag>, C>>::format(
    const urd::index_view<rp::ConcreteEffect<Family, rk::DlTag, FeatureTag, ObservationTag>, C>& value,
    fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "({} {})",
                          ObservationTag::keyword,
                          std::visit([](auto concrete) { return concrete.get_symbol(); }, value.get_feature().get_variant()));
}

fmt::format_context::iterator fmt::formatter<rp::ProofStatus>::format(rp::ProofStatus value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", rp::to_string(value)); }

template<rp::ProgramFamilyTag Family>
fmt::format_context::iterator fmt::formatter<rp::ProgramProofEdgeLabel<Family>>::format(const rp::ProgramProofEdgeLabel<Family>& value,
                                                                                        fmt::format_context& context) const
{
    if (value.rule)
        return fmt::format_to(context.out(), "rule={}", value.rule->get_symbol());
    return fmt::format_to(context.out(), "rule=<none>");
}

// Each family's records over the repositories that hold them: ext's repository holds the base family's records too.
template struct fmt::formatter<urd::index_view<rp::Feature<rk::BaseFamilyTag, rk::dl::BooleanTag>, rp::base::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::BaseFamilyTag, rk::dl::NumericalTag>, rp::base::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag>, rp::base::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag>, rp::base::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteCondition<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Positive>, rp::base::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteCondition<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Negative>, rp::base::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteCondition<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::EqualZero>, rp::base::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteCondition<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::GreaterZero>, rp::base::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Positive>, rp::base::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Negative>, rp::base::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Unchanged>, rp::base::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Increases>, rp::base::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Decreases>, rp::base::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Unchanged>, rp::base::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::BaseFamilyTag, rk::dl::BooleanTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::BaseFamilyTag, rk::dl::NumericalTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteCondition<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Positive>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteCondition<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Negative>, rp::ext::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteCondition<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::EqualZero>, rp::ext::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteCondition<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::GreaterZero>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Positive>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Negative>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Unchanged>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Increases>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Decreases>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::BaseFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Unchanged>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::ExtFamilyTag, rk::dl::ConceptTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::ExtFamilyTag, rk::dl::RoleTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::ExtFamilyTag, rk::dl::BooleanTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::ExtFamilyTag, rk::dl::NumericalTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::ExtFamilyTag, rp::dl::QueryFeature>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::ExtFamilyTag, rk::DlTag, rk::dl::ConceptTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::ExtFamilyTag, rk::DlTag, rk::dl::RoleTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::ExtFamilyTag, rk::DlTag, rk::dl::BooleanTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::ExtFamilyTag, rk::DlTag, rk::dl::NumericalTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::ExtFamilyTag, rk::DlTag, rp::dl::QueryFeature>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteCondition<rk::ExtFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Positive>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteCondition<rk::ExtFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Negative>, rp::ext::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteCondition<rk::ExtFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::EqualZero>, rp::ext::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteCondition<rk::ExtFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::GreaterZero>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::ExtFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Positive>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::ExtFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Negative>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::ExtFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Unchanged>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::ExtFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Increases>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::ExtFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Decreases>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::ExtFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Unchanged>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::IcpFamilyTag, rk::dl::ConceptTag>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::IcpFamilyTag, rk::dl::RoleTag>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::IcpFamilyTag, rk::dl::BooleanTag>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::IcpFamilyTag, rk::dl::NumericalTag>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::IcpFamilyTag, rp::dl::QueryFeature>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::IcpFamilyTag, rk::DlTag, rk::dl::ConceptTag>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::IcpFamilyTag, rk::DlTag, rk::dl::RoleTag>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::IcpFamilyTag, rk::DlTag, rk::dl::BooleanTag>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::IcpFamilyTag, rk::DlTag, rk::dl::NumericalTag>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::IcpFamilyTag, rk::DlTag, rp::dl::QueryFeature>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteCondition<rk::IcpFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Positive>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteCondition<rk::IcpFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Negative>, rp::icp::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteCondition<rk::IcpFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::EqualZero>, rp::icp::Repository>>;
template struct fmt::formatter<
    urd::index_view<rp::ConcreteCondition<rk::IcpFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::GreaterZero>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::IcpFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Positive>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::IcpFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Negative>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::IcpFamilyTag, rk::DlTag, rk::dl::BooleanTag, rp::dl::Unchanged>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::IcpFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Increases>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::IcpFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Decreases>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteEffect<rk::IcpFamilyTag, rk::DlTag, rk::dl::NumericalTag, rp::dl::Unchanged>, rp::icp::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Feature<rk::UnsFamilyTag, rk::dl::BooleanTag>, runir::kr::uns::Repository>>;
template struct fmt::formatter<urd::index_view<rp::ConcreteFeature<rk::UnsFamilyTag, rk::DlTag, rk::dl::BooleanTag>, runir::kr::uns::Repository>>;
template struct fmt::formatter<rp::ProgramProofEdgeLabel<rk::ExtFamilyTag>>;
template struct fmt::formatter<rp::ProgramProofEdgeLabel<rk::IcpFamilyTag>>;
