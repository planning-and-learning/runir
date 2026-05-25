#ifndef RUNIR_KR_PS_EXT_REGISTER_DATA_HPP_
#define RUNIR_KR_PS_EXT_REGISTER_DATA_HPP_

#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/ps/ext/register_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::ps::ext::Register>
{
    Index<runir::kr::ps::ext::Register> index;
    ::cista::offset::string name;
    runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> identifier;

    Data() = default;
    Data(::cista::offset::string name_, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> identifier_) :
        index(),
        name(std::move(name_)),
        identifier(identifier_)
    {
    }
    Data(const std::string& name_, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> identifier_) : index(), name(name_), identifier(identifier_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(name);
        tyr::clear(identifier);
    }

    auto cista_members() const noexcept { return std::tie(index, name, identifier); }
    auto identifying_members() const noexcept { return std::tie(name, identifier); }
};

}  // namespace tyr

#endif
