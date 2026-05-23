#ifndef RUNIR_KR_DL_ROLE_INDEX_HPP_
#define RUNIR_KR_DL_ROLE_INDEX_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <stdexcept>
#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(runir::kr::dl::FamilyRoleConstructorTag<Family, Tag> && !std::same_as<Tag, runir::kr::dl::RegisterIdentifierTag>)
struct Index<runir::kr::dl::Role<Family, Tag>> : IndexMixin<Index<runir::kr::dl::Role<Family, Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Role<Family, Tag>>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Index<runir::kr::dl::Role<Family, runir::kr::dl::RegisterIdentifierTag>> :
    IndexMixin<Index<runir::kr::dl::Role<Family, runir::kr::dl::RegisterIdentifierTag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Role<Family, runir::kr::dl::RegisterIdentifierTag>>>;

    constexpr Index() noexcept = default;
    explicit Index(uint_t value) : Base(check_bounds(value)) {}

private:
    static constexpr uint_t check_bounds(uint_t value)
    {
        if (value != Base::MAX && value >= runir::kr::dl::num_registers)
            throw std::out_of_range("Register identifier index is out of range.");
        return value;
    }
};

}  // namespace tyr

#endif
