#ifndef RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/grammar/role_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <utility>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NonemptyTag>>
{
    using ConstructorVariant = ::cista::offset::variant<Index<GrammarConceptChoice<Family>>, Index<GrammarRoleChoice<Family>>>;
    using Arg = ConstructorVariant;
    Index<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::NonemptyTag>> index;
    ConstructorVariant arg;
    Data() = default;
    explicit Data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}
    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
    }
    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

}  // namespace ygg

#endif
