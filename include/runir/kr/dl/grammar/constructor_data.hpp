#ifndef RUNIR_KR_DL_GRAMMAR_CONSTRUCTOR_DATA_HPP_
#define RUNIR_KR_DL_GRAMMAR_CONSTRUCTOR_DATA_HPP_

#include "runir/kr/dl/grammar/numerical_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace runir::kr::dl::grammar::detail
{

template<runir::kr::dl::CategoryTag Category>
struct ConstructorDataTraits;

template<>
struct ConstructorDataTraits<runir::kr::dl::ConceptTag>
{
    using Tags = runir::kr::dl::GenerationConceptConstructorTags;

    template<typename Tag>
    using Index = ygg::Index<Concept<Tag>>;
};

template<>
struct ConstructorDataTraits<runir::kr::dl::RoleTag>
{
    using Tags = runir::kr::dl::GenerationRoleConstructorTags;

    template<typename Tag>
    using Index = ygg::Index<Role<Tag>>;
};

template<>
struct ConstructorDataTraits<runir::kr::dl::BooleanTag>
{
    using Tags = runir::kr::dl::GenerationBooleanConstructorTags;

    template<typename Tag>
    using Index = ygg::Index<Boolean<Tag>>;
};

template<>
struct ConstructorDataTraits<runir::kr::dl::NumericalTag>
{
    using Tags = runir::kr::dl::GenerationNumericalConstructorTags;

    template<typename Tag>
    using Index = ygg::Index<Numerical<Tag>>;
};

template<runir::kr::dl::CategoryTag Category>
using ConstructorDataVariant =
    ygg::ApplyTypeListT<::cista::offset::variant,
                        ygg::MapTypeListT<ConstructorDataTraits<Category>::template Index, typename ConstructorDataTraits<Category>::Tags>>;

}  // namespace runir::kr::dl::grammar::detail

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::grammar::Constructor<Category>>
{
    using Variant = runir::kr::dl::grammar::detail::ConstructorDataVariant<Category>;

    Index<runir::kr::dl::grammar::Constructor<Category>> index;
    Variant variant;

    Data() = default;
    explicit Data(Variant variant_) : index(), variant(std::move(variant_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<Variant, C>;
    template<typename C>
    explicit Data(const ViewVariant<C>& variant_) :
        index(),
        variant(std::visit([](const auto& view) -> Variant { return Variant(view.get_index()); }, variant_))
    {
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::variant> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::variant> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

}  // namespace ygg

#endif
