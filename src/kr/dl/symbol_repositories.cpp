#include "runir/kr/detail/interning_contracts.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"
#include "runir/kr/dl/repository.hpp"

static_assert(runir::kr::detail::InternsData<runir::kr::dl::ConstructorRepository, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::TopTag>>);
static_assert(runir::kr::detail::rejects_symbol<runir::kr::dl::ConstructorRepository, runir::kr::dl::Concept<runir::kr::dl::grammar::GrammarFamily, runir::kr::dl::TopTag>>());
