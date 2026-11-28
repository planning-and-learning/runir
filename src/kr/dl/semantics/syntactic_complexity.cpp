#include "runir/kr/dl/semantics/syntactic_complexity.hpp"

#include "runir/kr/dl/repository.hpp"

#include <cstddef>
#include <variant>

namespace runir::kr::dl::semantics
{

namespace
{

template<typename View>
std::size_t of_variant(View view)
{
    return std::visit([](auto child) { return syntactic_complexity(child); }, view.variant());
}

}  // namespace

std::size_t syntactic_complexity(const ConstructorView<ConceptTag>& view) { return of_variant(view); }

std::size_t syntactic_complexity(const ConstructorView<RoleTag>& view) { return of_variant(view); }

std::size_t syntactic_complexity(const ConstructorView<BooleanTag>& view) { return of_variant(view); }

std::size_t syntactic_complexity(const ConstructorView<NumericalTag>& view) { return of_variant(view); }

std::size_t syntactic_complexity(const QueryView<>& view) { return of_variant(view); }

}  // namespace runir::kr::dl::semantics
