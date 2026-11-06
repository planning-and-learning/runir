#ifndef RUNIR_KR_DL_CONSTRUCTOR_DATA_HPP_
#define RUNIR_KR_DL_CONSTRUCTOR_DATA_HPP_

#include "runir/kr/dl/numerical_data.hpp"
#include "runir/kr/dl/query_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>

namespace runir::kr::dl::detail
{

template<CategoryTag Category>
struct ConstructorDataTraits;

template<>
struct ConstructorDataTraits<ConceptTag>
{
    using Tags = ConceptConstructorTags;

    template<typename Tag>
    using Index = urd::index<Concept<Tag>>;

    using Indices = boost::mp11::mp_append<boost::mp11::mp_transform<Index, Tags>, boost::mp11::mp_list<urd::index<QueryProjection<ConceptTag>>>>;
};

template<>
struct ConstructorDataTraits<RoleTag>
{
    using Tags = RoleConstructorTags;

    template<typename Tag>
    using Index = urd::index<Role<Tag>>;

    using Indices = boost::mp11::mp_append<boost::mp11::mp_transform<Index, Tags>, boost::mp11::mp_list<urd::index<QueryProjection<RoleTag>>>>;
};

template<>
struct ConstructorDataTraits<BooleanTag>
{
    using Tags = BooleanConstructorTags;

    template<typename Tag>
    using Index = urd::index<Boolean<Tag>>;

    using Indices = boost::mp11::mp_transform<Index, Tags>;
};

template<>
struct ConstructorDataTraits<NumericalTag>
{
    using Tags = NumericalConstructorTags;

    template<typename Tag>
    using Index = urd::index<Numerical<Tag>>;

    using Indices = boost::mp11::mp_transform<Index, Tags>;
};

template<CategoryTag Category>
using ConstructorDataVariant = boost::mp11::mp_apply<::cista::offset::variant, typename ConstructorDataTraits<Category>::Indices>;

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
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::variant> {}; }
};

}  // namespace urd

#endif
