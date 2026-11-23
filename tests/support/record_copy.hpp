#ifndef RUNIR_TESTS_SUPPORT_RECORD_COPY_HPP_
#define RUNIR_TESTS_SUPPORT_RECORD_COPY_HPP_

#include <urd/elements/clone.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/elements/data.hpp>

namespace runir::tests
{

/// A copy of a move-only record, built from its fields (urd::Fields): its index stays empty and the members that are
/// not fields (derived during insertion) keep their defaults.
template<typename T>
urd::data<T> copy_fields(const urd::data<T>& source)
{
    auto result = urd::data<T>();
    urd::for_each_field<urd::data<T>>([&]<typename F>() { result.*F::member = urd::clone(source.*F::member); });
    return result;
}

}  // namespace runir::tests

#endif
