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

struct UnsFamilyTag
{
};

template<typename T>
concept FamilyTag = std::same_as<T, BaseFamilyTag> || std::same_as<T, ExtFamilyTag> || std::same_as<T, UnsFamilyTag>;

}  // namespace runir::kr

#endif
