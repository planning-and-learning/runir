#include "module.hpp"

#include <runir/kr/uns/repository.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::uns::dl
{

void bind_indices(nb::module_& m) { ygg::bind_index<ygg::Index<runir::kr::uns::dl::Feature>>(m, "FeatureIndex"); }

}  // namespace runir::kr::uns::dl
