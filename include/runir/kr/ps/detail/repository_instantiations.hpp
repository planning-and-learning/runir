#ifndef RUNIR_KR_PS_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_PS_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of intern for every symbol type of the program repositories (base, ext, icp, uns: ext holds base's 16)
/// and of the execution repositories (ext, icp; ground and lifted), instantiated in the library (src/kr/ps/repository.cpp).
/// Include only from sources that intern many of these types; see yggdrasil's
/// docs/TEMPLATE_RULES.md, rule 9.

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"
#include "runir/kr/ps/icp/declarations.hpp"
#include "runir/kr/ps/icp/execution_declarations.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <yggdrasil/formalism/intern_instantiations.hpp>

/// The instantiations (PREFIX extern: declarations).
#define RUNIR_KR_PS_INTERNS(PREFIX)                                                                                                      \
    YGG_FORMALISM_INTERNS(PREFIX,                                                                                               \
                          86,                                                                                                   \
                          false,                                                                                                \
                          boost::mp11::mp_unique<boost::mp11::mp_append<runir::kr::ps::base::RepositoryTypes,                   \
                                                                        runir::kr::ps::ext::RepositoryTypes,                    \
                                                                        runir::kr::ps::icp::RepositoryTypes,                    \
                                                                        runir::kr::uns::RepositoryTypes>>)                      \
    YGG_FORMALISM_INTERNS(PREFIX,                                                                                               \
                          8,                                                                                                    \
                          false,                                                                                                \
                          boost::mp11::mp_unique<boost::mp11::mp_append<runir::kr::ps::ext::ExecutionRepositoryTypes<tyr::GroundTag>, \
                                                                        runir::kr::ps::ext::ExecutionRepositoryTypes<tyr::LiftedTag>, \
                                                                        runir::kr::ps::icp::ExecutionRepositoryTypes<tyr::GroundTag>, \
                                                                        runir::kr::ps::icp::ExecutionRepositoryTypes<tyr::LiftedTag>>>)

RUNIR_KR_PS_INTERNS(extern)

#endif
