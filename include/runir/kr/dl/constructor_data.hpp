#ifndef RUNIR_KR_DL_CONSTRUCTOR_DATA_HPP_
#define RUNIR_KR_DL_CONSTRUCTOR_DATA_HPP_

#include "runir/kr/dl/numerical_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace runir::kr::dl::detail
{

template<FamilyTag Family, CategoryTag Category>
struct ConstructorDataTraits;

template<FamilyTag Family>
struct ConstructorDataTraits<Family, ConceptTag>
{
    using Tags = FamilyConceptConstructorTags<Family>;

    template<typename Tag>
    using Index = ygg::Index<Concept<Family, Tag>>;
};

template<FamilyTag Family>
struct ConstructorDataTraits<Family, RoleTag>
{
    using Tags = FamilyRoleConstructorTags<Family>;

    template<typename Tag>
    using Index = ygg::Index<Role<Family, Tag>>;
};

template<FamilyTag Family>
struct ConstructorDataTraits<Family, BooleanTag>
{
    using Tags = FamilyBooleanConstructorTags<Family>;

    template<typename Tag>
    using Index = ygg::Index<Boolean<Family, Tag>>;
};

template<FamilyTag Family>
struct ConstructorDataTraits<Family, NumericalTag>
{
    using Tags = FamilyNumericalConstructorTags<Family>;

    template<typename Tag>
    using Index = ygg::Index<Numerical<Family, Tag>>;
};

template<FamilyTag Family, CategoryTag Category>
using ConstructorDataVariant =
    ygg::ApplyTypeListT<::cista::offset::variant,
                        ygg::MapTypeListT<ConstructorDataTraits<Family, Category>::template Index, typename ConstructorDataTraits<Family, Category>::Tags>>;

}  // namespace runir::kr::dl::detail

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::Constructor<Family, Category>>
{
    using Variant = runir::kr::dl::detail::ConstructorDataVariant<Family, Category>;

    Index<runir::kr::dl::Constructor<Family, Category>> index;
    Variant value;

    Data() = default;
    explicit Data(Variant value_) : index(), value(std::move(value_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace ygg

#endif
