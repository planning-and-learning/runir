#ifndef RUNIR_KR_PS_EXT_DL_AST_AST_HPP_
#define RUNIR_KR_PS_EXT_DL_AST_AST_HPP_

#include "runir/kr/ps/base/dl/ast/ast.hpp"

namespace runir::kr::ps::base::dl::ast
{

template<>
struct Feature<runir::kr::ExtFamilyTag> :
    PositionedVariant<Concept<runir::kr::ExtFamilyTag>, BooleanFeature<runir::kr::ExtFamilyTag>, NumericalFeature<runir::kr::ExtFamilyTag>>
{
    using Base = PositionedVariant<Concept<runir::kr::ExtFamilyTag>, BooleanFeature<runir::kr::ExtFamilyTag>, NumericalFeature<runir::kr::ExtFamilyTag>>;
    using Base::Base;
    using Base::operator=;
};

}  // namespace runir::kr::ps::base::dl::ast

#endif
