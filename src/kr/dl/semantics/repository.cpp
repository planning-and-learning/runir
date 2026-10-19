#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <yggdrasil/database/syntax/query_repository.hpp>

template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::RegisterValues>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::semantics::CallArguments>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Columns<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int> >>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Relation<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int> >>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, void>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryInputTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryEmptyTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryJoinTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryProjectTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryRenameTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QuerySelectEqualTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QuerySelectValueTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryUnionTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryDifferenceTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryDistanceTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryGenericJoinTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QuerySemijoinTag>>;
template class ygg::formalism::BasicSymbolRepository<ygg::database::Query<ygg::TypeList<ygg::Index<ygg::formalism::Object<tyr::formalism::ObjectTag> >, unsigned int>, ygg::database::QueryAntijoinTag>>;
