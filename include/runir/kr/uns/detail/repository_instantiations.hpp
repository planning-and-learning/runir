#ifndef RUNIR_KR_UNS_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_UNS_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of the symbol storage of every uns symbol type, instantiated in the library.
/// Include only from sources that intern many of these types; see yggdrasil's docs/TEMPLATE_RULES.md.

#include "runir/kr/uns/repository.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::uns::ClassifierLiteral>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::uns::ClassifierClause>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::uns::Classifier>;

#else

#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

#endif

#endif
