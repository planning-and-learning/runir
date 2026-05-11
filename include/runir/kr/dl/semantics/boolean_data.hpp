#ifndef RUNIR_SEMANTICS_BOOLEAN_DATA_HPP_
#define RUNIR_SEMANTICS_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>>
{
    using ConstructorVariant =
        ::cista::offset::variant<Index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>, Index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>>;

    Index<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>> index;
    ConstructorVariant arg;

    Data() = default;
    explicit Data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

static_assert(!uses_trivial_storage_v<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>>);

}

#endif
