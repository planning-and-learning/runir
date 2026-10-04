#include "../../query_bindings.hpp"
#include "bindings.hpp"

namespace runir::kr::dl::base
{

void bind_semantics_query(nb::module_& m)
{
    ygg::bind_index<ygg::Index<QueryColumn>>(m, "QueryColumnIndex");
    auto data = nb::class_<ygg::Data<QueryColumn>>(m, "QueryColumnData")
                    .def(nb::init<>())
                    .def_rw("index", &ygg::Data<QueryColumn>::index)
                    .def_rw("name", &ygg::Data<QueryColumn>::name);
    ygg::add_comparison(data);
    python::bind_queries<runir::kr::BaseFamilyTag>(m);
}

}  // namespace runir::kr::dl::base
