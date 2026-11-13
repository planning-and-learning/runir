#ifndef RUNIR_SERIALIZATION_TYPES_FIELDS_HPP_
#define RUNIR_SERIALIZATION_TYPES_FIELDS_HPP_

#include <type_traits>
#include <urd/elements/concepts.hpp>
#include <urd/elements/view.hpp>

namespace ygg::serialization
{

/// A record by its fields (urd::Fields), in order: each under its name, read through the view (resolved through its
/// context). A record whose serialization differs (a variant record) has an overload of its own, which is more
/// specialized.
template<typename Archive, typename T, typename C>
    requires urd::HasFields<urd::data<T>>
void describe_fields(Archive& ar, std::type_identity<urd::index_view<T, C>> /*type*/)
{
    urd::for_each_field<urd::data<T>>([&]<typename F>()
                                      { ar.field(F::name, [](const auto& value) -> decltype(auto) { return value.template member<F::member>(); }); });
}

}

#endif
