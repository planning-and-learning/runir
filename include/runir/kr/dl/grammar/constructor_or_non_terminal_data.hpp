#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_DATA_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_DATA_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>>
{
    using Variant = ::ygg::IndexVariant<ygg::TypeList<runir::kr::dl::grammar::Constructor<Category>, runir::kr::dl::grammar::NonTerminal<Category>>>;
    Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>> index;
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
