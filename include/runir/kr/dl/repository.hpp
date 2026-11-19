#ifndef RUNIR_REPOSITORY_HPP_
#define RUNIR_REPOSITORY_HPP_

#include "runir/kr/dl/argument_builder.hpp"
#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/boolean_builder.hpp"
#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/concept_builder.hpp"
#include "runir/kr/dl/construction_metadata.hpp"
#include "runir/kr/dl/constructor_builder.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/detail/constructor_repository.hpp"
#include "runir/kr/dl/numerical_builder.hpp"
#include "runir/kr/dl/query_builder.hpp"
#include "runir/kr/dl/query_construction.hpp"
#include "runir/kr/dl/register_builder.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/dl/role_builder.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"

#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::dl
{

using Builder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, boost::mp11::mp_push_front<ConstructorRepositoryTypes, ConstructorRepository>>;

using runir::kr::dl::detail::get_repository;
using ygg::formalism::checkout;

template<typename T>
    requires ygg::formalism::SupportsSymbol<ConstructorRepository, T>
void prepare_for_insert(ConstructorRepository& repository, urd::data<T>& data)
{
    canonicalize(data);
    detail::prepare(data, repository);
}

using ygg::formalism::insert;
}

#endif
