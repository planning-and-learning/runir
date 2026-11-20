#ifndef RUNIR_KR_PS_ICP_XCONDITION_BUILDER_HPP_
#define RUNIR_KR_PS_ICP_XCONDITION_BUILDER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/icp/xcondition_data.hpp"
#include "runir/kr/ps/icp/xcondition_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of XCondition: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::icp::XCondition, C>
{
    urd::index<runir::kr::ps::icp::XCondition> index;
    urd::built_t<runir::kr::ps::icp::ConditionOperation, C> operation {};
    urd::built_t<urd::flat_variant<runir::kr::ps::icp::ArgumentPosition, urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>>,
                 ::runir::kr::dl::ConstructorRepository>
        object;
    urd::built_t<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>, C> feature;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operation", &builder::operation>, urd::field<"object", &builder::object>, urd::field<"feature", &builder::feature>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operation, &builder::object, &builder::feature> {}; }
};

}  // namespace urd

#endif
