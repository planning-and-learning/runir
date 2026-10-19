#ifndef RUNIR_KR_DL_SEMANTICS_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_DL_SEMANTICS_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of the symbol storage of every denotation, relation and query type, instantiated in the library.
/// Include only from sources that intern many of these types; see yggdrasil's docs/TEMPLATE_RULES.md.

#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <yggdrasil/database/syntax/query_repository.hpp>

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::RegisterValues>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::CallArguments>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Columns<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int> >>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Relation<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int> >>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, void>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryInputTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryEmptyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryJoinTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryProjectTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryRenameTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QuerySelectEqualTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QuerySelectValueTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryUnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryDifferenceTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryDistanceTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryGenericJoinTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QuerySemijoinTag>>;
extern template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryAntijoinTag>>;

#endif

#endif
