#ifndef RUNIR_KR_DL_CONSTRUCTOR_DATA_HPP_
#define RUNIR_KR_DL_CONSTRUCTOR_DATA_HPP_

#include "runir/kr/dl/numerical_data.hpp"
#include "runir/kr/dl/query_data.hpp"

#include <boost/mp11.hpp>
#include <cista/containers/variant.h>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>
#include <variant>

namespace runir::kr::dl::detail
{

/// A semantic constructor of Category: one of its records, or (concepts and roles) a query projection.
template<CategoryTag Category>
using ConstructorDataVariant = urd::index_variant<boost::mp11::mp_append<RecordTypes<SemanticFamily, Category>,
                                                                         by_category<Category,
                                                                                     boost::mp11::mp_list<QueryProjection<ConceptTag>>,
                                                                                     boost::mp11::mp_list<QueryProjection<RoleTag>>,
                                                                                     boost::mp11::mp_list<>,
                                                                                     boost::mp11::mp_list<>>>>;

}  // namespace runir::kr::dl::detail

namespace urd
{

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::dl::Constructor<Category>>
{
    using Variant = runir::kr::dl::detail::ConstructorDataVariant<Category>;

    urd::index<runir::kr::dl::Constructor<Category>> index;
    Variant variant;
    bool is_static = false;

    data() = default;
    // is_static is derived during insertion and therefore not a constructor parameter.
    explicit data(Variant variant_) : index(), variant(std::move(variant_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::variant, &data::is_static> {}; }
    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"variant", &data::variant>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
