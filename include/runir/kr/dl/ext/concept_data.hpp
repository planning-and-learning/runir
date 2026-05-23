#ifndef RUNIR_KR_DL_EXT_CONCEPT_DATA_HPP_
#define RUNIR_KR_DL_EXT_CONCEPT_DATA_HPP_

#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/dl/ext/register_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterTag>>
{
    Index<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterTag>> index;
    Index<runir::kr::dl::RegisterTag> identifier;

    Data() = default;
    explicit Data(Index<runir::kr::dl::RegisterTag> identifier_) : index(), identifier(identifier_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(identifier);
    }

    auto cista_members() const noexcept { return std::tie(index, identifier); }
    auto identifying_members() const noexcept { return std::tie(identifier); }
};

}  // namespace tyr

#endif
