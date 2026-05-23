#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_EXT_AST_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_EXT_AST_HPP_

#include "runir/kr/dl/grammar/parser/base/ast.hpp"

namespace runir::kr::dl::grammar::parser::ext::ast
{
namespace x3 = boost::spirit::x3;

struct ConceptRegister : x3::position_tagged
{
    static constexpr auto keyword = "c_register";

    tyr::uint_t identifier;
};

}  // namespace runir::kr::dl::grammar::parser::ext::ast

namespace runir::kr::dl::grammar::parser::base::ast
{

template<>
struct Constructor<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ConceptTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<ConceptBot>,
                x3::forward_ast<ConceptTop>,
                x3::forward_ast<ConceptAtomicState>,
                x3::forward_ast<ConceptAtomicGoal>,
                x3::forward_ast<ConceptIntersection<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptUnion<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptNegation<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptValueRestriction<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptExistentialQuantification<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptAtLeastNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptAtMostNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptExactNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptQualifiedAtLeastNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptQualifiedAtMostNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptQualifiedExactNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptRoleValueMap<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptAgreement<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptRoleFillers<runir::kr::dl::ExtFamilyTag>>,
                x3::forward_ast<ConceptOneOf>,
                x3::forward_ast<ConceptNominal>,
                x3::forward_ast<runir::kr::dl::grammar::parser::ext::ast::ConceptRegister>>
{
    using Base = x3::variant<x3::forward_ast<ConceptBot>,
                             x3::forward_ast<ConceptTop>,
                             x3::forward_ast<ConceptAtomicState>,
                             x3::forward_ast<ConceptAtomicGoal>,
                             x3::forward_ast<ConceptIntersection<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptUnion<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptNegation<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptValueRestriction<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptExistentialQuantification<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptAtLeastNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptAtMostNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptExactNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptQualifiedAtLeastNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptQualifiedAtMostNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptQualifiedExactNumberRestriction<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptRoleValueMap<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptAgreement<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptRoleFillers<runir::kr::dl::ExtFamilyTag>>,
                             x3::forward_ast<ConceptOneOf>,
                             x3::forward_ast<ConceptNominal>,
                             x3::forward_ast<runir::kr::dl::grammar::parser::ext::ast::ConceptRegister>>;
    using Base::Base;
    using Base::operator=;
};

}  // namespace runir::kr::dl::grammar::parser::base::ast

#endif
