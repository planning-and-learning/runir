#ifndef RUNIR_KR_DL_REGISTER_DATA_HPP_
#define RUNIR_KR_DL_REGISTER_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <urd/elements/flat.hpp>

namespace urd
{

template<runir::kr::dl::ConceptOrRoleTag Category>
struct data<runir::kr::dl::Register<Category>>
{
    urd::index<runir::kr::dl::Register<Category>> index;
    urd::flat_string name;
    runir::kr::dl::RegisterIdentifier<Category> identifier;

    data() = default;
    data(urd::flat_string name_, runir::kr::dl::RegisterIdentifier<Category> identifier_) : index(), name(std::move(name_)), identifier(identifier_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name, &data::identifier> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name, &data::identifier> {}; }
};

}  // namespace urd

#endif
