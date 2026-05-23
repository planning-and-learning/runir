#ifndef RUNIR_KR_DECLARATIONS_HPP_
#define RUNIR_KR_DECLARATIONS_HPP_

#include <concepts>

namespace runir::kr
{

struct DlTag
{
};

struct BaseFamilyTag
{
};

struct ExtFamilyTag
{
};

template<typename T>
concept FamilyTag = std::same_as<T, BaseFamilyTag> || std::same_as<T, ExtFamilyTag>;

}  // namespace runir::kr

#endif
