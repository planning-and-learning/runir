#ifndef RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_

#include "runir/kr/ps/ext/execution_data.hpp"

#include <tyr/planning/declarations.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
class ExecutionBuilder
{
private:
    ygg::formalism::BuilderStorage<RegisterValues, CallArguments, CallStack, ExecutionState<Kind>> m_storage;

public:
    template<typename T>
    [[nodiscard]] auto get_builder()
    {
        auto builder = m_storage.template get_builder<T>();
        builder->clear();
        return builder;
    }
};

}  // namespace runir::kr::ps::ext

#endif
