#include "bindings.hpp"

#include <runir/kr/dl/argument_data.hpp>
#include <runir/kr/dl/argument_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::ext
{

namespace
{

template<CategoryTag Category>
void bind_argument_data(nb::module_& m, const char* name)
{
    using Type = Argument<Category>;
    using Data = ygg::Data<Type>;

    auto data = nb::class_<Data>(m, name)
                    .def(nb::init<>())
                    .def_rw("index", &Data::index)
                    .def_rw("name", &Data::name)
                    .def_rw("identifier", &Data::identifier);
    ygg::add_comparison(data);
}

template<CategoryTag Category>
void bind_argument_view(nb::module_& m, const char* name)
{
    using Type = Argument<Category>;
    using View = ygg::View<ygg::Index<Type>, ExtConstructorRepository>;
    auto view = nb::class_<View>(m, name)
                    .def("get_index", &View::get_index)
                    .def("get_name", &View::get_name)
                    .def("get_identifier", &View::get_identifier);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
}

}  // namespace

void bind_argument(nb::module_& m)
{
    using Concept = Argument<ConceptTag>;
    using Role = Argument<RoleTag>;
    using Boolean = Argument<BooleanTag>;
    using Numerical = Argument<NumericalTag>;

    ygg::bind_index<ygg::Index<Concept>>(m, "ConceptArgumentIndex");
    ygg::bind_index<ygg::Index<Role>>(m, "RoleArgumentIndex");
    ygg::bind_index<ygg::Index<Boolean>>(m, "BooleanArgumentIndex");
    ygg::bind_index<ygg::Index<Numerical>>(m, "NumericalArgumentIndex");
    ygg::bind_index<ArgumentIdentifier<ConceptTag>>(m, "ConceptArgumentIdentifier");
    ygg::bind_index<ArgumentIdentifier<RoleTag>>(m, "RoleArgumentIdentifier");
    ygg::bind_index<ArgumentIdentifier<BooleanTag>>(m, "BooleanArgumentIdentifier");
    ygg::bind_index<ArgumentIdentifier<NumericalTag>>(m, "NumericalArgumentIdentifier");

    bind_argument_data<ConceptTag>(m, "ConceptArgumentData");
    bind_argument_data<RoleTag>(m, "RoleArgumentData");
    bind_argument_data<BooleanTag>(m, "BooleanArgumentData");
    bind_argument_data<NumericalTag>(m, "NumericalArgumentData");
    bind_argument_view<ConceptTag>(m, "ConceptArgument");
    bind_argument_view<RoleTag>(m, "RoleArgument");
    bind_argument_view<BooleanTag>(m, "BooleanArgument");
    bind_argument_view<NumericalTag>(m, "NumericalArgument");
}

}  // namespace runir::kr::dl::ext
