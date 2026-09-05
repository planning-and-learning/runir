#ifndef RUNIR_KR_UNS_CANONICALIZATION_HPP_
#define RUNIR_KR_UNS_CANONICALIZATION_HPP_

#include "runir/kr/ps/canonicalization.hpp"
#include "runir/kr/uns/classifier_data.hpp"

#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::uns
{

inline void canonicalize(ygg::Data<ClassifierLiteral>&) noexcept {}

inline void canonicalize(ygg::Data<ClassifierClause>& data) { ygg::canonicalize(data.literals); }

inline void canonicalize(ygg::Data<Classifier>& data)
{
    ygg::canonicalize(data.features);
    ygg::canonicalize(data.clauses);
}

}  // namespace runir::kr::uns

#endif
