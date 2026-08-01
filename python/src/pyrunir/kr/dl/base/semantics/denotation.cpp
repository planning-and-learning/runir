#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/denotation_view.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::base
{
namespace
{

template<CategoryTag Category>
void bind_denotation_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::semantics::Denotation<Category>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::semantics::DenotationRepository>;

    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);

    if constexpr (std::same_as<Category, BooleanTag> || std::same_as<Category, NumericalTag>)
        cls.def("get", [](View view) { return view.get(); });
}

}  // namespace

void bind_semantics_denotation(nb::module_& m)
{
    ygg::bind_index<ygg::Index<runir::kr::dl::semantics::Denotation<BooleanTag>>>(m, "BooleanDenotationIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::semantics::Denotation<NumericalTag>>>(m, "NumericalDenotationIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::semantics::Denotation<ConceptTag>>>(m, "ConceptDenotationIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::semantics::Denotation<RoleTag>>>(m, "RoleDenotationIndex");

    bind_denotation_view<BooleanTag>(m, "BooleanDenotation");
    bind_denotation_view<NumericalTag>(m, "NumericalDenotation");
    bind_denotation_view<ConceptTag>(m, "ConceptDenotation");
    bind_denotation_view<RoleTag>(m, "RoleDenotation");
}

}  // namespace runir::kr::dl::base
