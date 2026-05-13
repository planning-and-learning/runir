#include "module.hpp"

#include <runir/datasets/config.hpp>

namespace runir::datasets
{

void bind_config(nb::module_& m)
{
    nb::enum_<StateGraphCostMode>(m, "StateGraphCostMode")
        .value("UNIT_COST", StateGraphCostMode::UNIT_COST)
        .value("ACTION_COST", StateGraphCostMode::ACTION_COST);
}

}  // namespace runir::datasets
