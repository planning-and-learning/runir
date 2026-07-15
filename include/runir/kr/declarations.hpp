#ifndef RUNIR_KR_DECLARATIONS_HPP_
#define RUNIR_KR_DECLARATIONS_HPP_

#include <concepts>
#include <memory>
#include <tyr/declarations.hpp>

namespace runir::kr
{

template<tyr::TaskKind Kind>
struct TaskContext;

template<tyr::TaskKind Kind>
using TaskContextPtr = std::shared_ptr<TaskContext<Kind>>;

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
