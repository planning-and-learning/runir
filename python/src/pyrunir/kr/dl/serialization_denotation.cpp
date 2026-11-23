#include "module.hpp"
#include "pyrunir/kr/dl/serialization.hpp"

#include <runir/kr/dl/semantics/call_arguments_view.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/denotation_view.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>

namespace runir::kr::dl
{
void bind_denotation_serialization()
{
    using DenotationViews =
        boost::mp11::mp_append<boost::mp11::mp_transform<runir::kr::dl::semantics::DenotationView, runir::kr::dl::CategoryTags>,
                               boost::mp11::mp_list<runir::kr::dl::semantics::RegisterValuesView, runir::kr::dl::semantics::CallArgumentsView>>;
    ygg::python::bind_serialization(DenotationViews {});
}
}  // namespace runir::kr::dl
