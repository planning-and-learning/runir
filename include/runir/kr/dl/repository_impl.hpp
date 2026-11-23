#ifndef RUNIR_KR_DL_REPOSITORY_IMPL_HPP_
#define RUNIR_KR_DL_REPOSITORY_IMPL_HPP_

// How the repository prepares a record for inserting it (prepare_for_insert): seen only by the sources that instantiate
// the repository's inserts and finds (src/kr/dl/repository*.cpp), which its header declares (repository.hpp), so that
// the query plans' construction (query_construction.hpp) stays out of the files that only read and insert through it.

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/query_construction.hpp"
#include "runir/kr/dl/repository.hpp"

namespace runir::kr::dl
{

template<typename T>
    requires ygg::formalism::SupportsSymbol<ConstructorRepository, T>
void prepare_for_insert(const ConstructorRepository& repository, urd::data<T>& data)
{
    using ygg::formalism::canonicalize;  // the default, for a kind without a canonical form of its own
    canonicalize(data);
    detail::prepare(data, repository);
}

}

#endif
