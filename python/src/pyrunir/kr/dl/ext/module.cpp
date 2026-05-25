#include "module.hpp"

namespace runir::kr::dl::ext
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_datas(m);
    bind_parser(m);
    bind_repository(m);
}

}  // namespace runir::kr::dl::ext
