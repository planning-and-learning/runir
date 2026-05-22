#ifndef RUNIR_KR_MPG_DL_CONCEPT_DATA_HPP_
#define RUNIR_KR_MPG_DL_CONCEPT_DATA_HPP_

#include "runir/kr/mpg/dl/concept_index.hpp"
#include "runir/kr/mpg/register_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::mpg::dl::Concept<runir::kr::mpg::dl::RegisterTag>>
{
    Index<runir::kr::mpg::dl::Concept<runir::kr::mpg::dl::RegisterTag>> index;
    Index<runir::kr::mpg::Register> reg;

    Data() = default;
    Data(Index<runir::kr::mpg::Register> reg_) : index(), reg(reg_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(reg);
    }

    auto cista_members() const noexcept { return std::tie(index, reg); }
    auto identifying_members() const noexcept { return std::tie(reg); }
};

}  // namespace tyr

#endif
