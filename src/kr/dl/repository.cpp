#include "runir/kr/dl/repository_impl.hpp"

#include <cstddef>
#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_impl.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

// The repository's symbols and its builder, which the library's other sources only declare
// (yggdrasil/formalism/symbol_context.hpp, builder.hpp): their construction here, a range of the symbols' keys and the
// builder's types in each of repository_{0,1,2,3}.cpp, so that no one source holds them all (brokkr's template rule T5).
// The count the sources' ranges cover: the repository's symbols and its builder's types alike.
constexpr auto kind_count = std::size_t { 114 };
static_assert(boost::mp11::mp_size<ygg::formalism::detail::SymbolContextOf<runir::kr::dl::ConstructorRepository>::SymbolTypes>::value == kind_count);
static_assert(boost::mp11::mp_size<runir::kr::dl::Builder::Types>::value == kind_count);
YGG_FORMALISM_SYMBOL_CONTEXT_MEMBERS(, runir::kr::dl::ConstructorRepository)
YGG_FORMALISM_BUILDER_MEMBERS(, runir::kr::dl::Builder)
