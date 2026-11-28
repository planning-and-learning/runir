#ifndef RUNIR_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/record_view.hpp"

#include <cstddef>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace runir::kr::dl::semantics
{
/// The syntactic complexity of a constructor of each category (defined in syntactic_complexity.cpp), declared before
/// its view, whose functions() list it. Each visits its operands, of every kind in turn, so the library compiles it once
/// instead of every file that measures one (brokkr's template rules, rule 24).
std::size_t syntactic_complexity(const runir::kr::dl::ConstructorView<runir::kr::dl::ConceptTag>& view);
std::size_t syntactic_complexity(const runir::kr::dl::ConstructorView<runir::kr::dl::RoleTag>& view);
std::size_t syntactic_complexity(const runir::kr::dl::ConstructorView<runir::kr::dl::BooleanTag>& view);
std::size_t syntactic_complexity(const runir::kr::dl::ConstructorView<runir::kr::dl::NumericalTag>& view);
}  // namespace runir::kr::dl::semantics

namespace urd
{
template<runir::kr::dl::CategoryTag Category, typename V>
struct view_mixin<runir::kr::dl::Constructor<Category>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto variant() const noexcept { return self().template member<&urd::record_of<V>::variant>(); }

    [[nodiscard]] bool is_static() const noexcept { return self().template member<&urd::record_of<V>::is_static>(); }

    static constexpr auto functions() noexcept
    {
        return urd::Functions<
            urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::dl::semantics::syntactic_complexity)>> {};
    }
};
}  // namespace urd

#endif
