#ifndef RUNIR_KR_DL_CONSTRUCTOR_DATA_HPP_
#define RUNIR_KR_DL_CONSTRUCTOR_DATA_HPP_

#include "runir/kr/dl/numerical_data.hpp"
#include "runir/kr/dl/query_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace runir::kr::dl::detail
{

template<CategoryTag Category>
struct ConstructorDataTraits;

template<>
struct ConstructorDataTraits<ConceptTag>
{
    using Tags = ConceptConstructorTags;

    template<typename Tag>
    using Index = ygg::Index<Concept<Tag>>;

    using Indices = ygg::ConcatTypeListsT<ygg::MapTypeListT<Index, Tags>, ygg::TypeList<ygg::Index<QueryProjection<ConceptTag>>>>;
};

template<>
struct ConstructorDataTraits<RoleTag>
{
    using Tags = RoleConstructorTags;

    template<typename Tag>
    using Index = ygg::Index<Role<Tag>>;

    using Indices = ygg::ConcatTypeListsT<ygg::MapTypeListT<Index, Tags>, ygg::TypeList<ygg::Index<QueryProjection<RoleTag>>>>;
};

template<>
struct ConstructorDataTraits<BooleanTag>
{
    using Tags = BooleanConstructorTags;

    template<typename Tag>
    using Index = ygg::Index<Boolean<Tag>>;

    using Indices = ygg::MapTypeListT<Index, Tags>;
};

template<>
struct ConstructorDataTraits<NumericalTag>
{
    using Tags = NumericalConstructorTags;

    template<typename Tag>
    using Index = ygg::Index<Numerical<Tag>>;

    using Indices = ygg::MapTypeListT<Index, Tags>;
};

template<CategoryTag Category>
using ConstructorDataVariant = ygg::ApplyTypeListT<::cista::offset::variant, typename ConstructorDataTraits<Category>::Indices>;

}  // namespace runir::kr::dl::detail

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::Constructor<Category>>
{
    using Variant = runir::kr::dl::detail::ConstructorDataVariant<Category>;

    Index<runir::kr::dl::Constructor<Category>> index;
    Variant variant;
    bool is_static = false;

    Data() = default;
    // is_static is derived during insertion and therefore not a constructor parameter.
    explicit Data(Variant variant_) : index(), variant(std::move(variant_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<Variant, C>;
    template<typename C>
    explicit Data(const ViewVariant<C>& variant_) :
        index(),
        variant(std::visit([](const auto& view) -> Variant { return Variant(view.get_index()); }, variant_))
    {
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::variant, &Data::is_static> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::variant> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

}  // namespace ygg

#endif
