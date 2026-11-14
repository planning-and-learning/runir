#include "runir/kr/dl/semantics/formatter.hpp"

#include "runir/kr/dl/repository.hpp"

#include <fmt/format.h>
#include <string>

namespace runir::kr::dl::semantics
{

std::string text(ConstructorView<ConceptTag> view) { return fmt::format("{}", view.get_variant()); }
std::string text(ConstructorView<RoleTag> view) { return fmt::format("{}", view.get_variant()); }
std::string text(ConstructorView<BooleanTag> view) { return fmt::format("{}", view.get_variant()); }
std::string text(ConstructorView<NumericalTag> view) { return fmt::format("{}", view.get_variant()); }
std::string text(QueryView<> view) { return fmt::format("{}", view.get_variant()); }

}  // namespace runir::kr::dl::semantics
