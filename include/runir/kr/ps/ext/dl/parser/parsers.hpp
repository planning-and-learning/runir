#ifndef RUNIR_KR_PS_EXT_DL_PARSER_PARSERS_HPP_
#define RUNIR_KR_PS_EXT_DL_PARSER_PARSERS_HPP_

#include "runir/kr/ps/ext/dl/ast/ast.hpp"

#include <boost/spirit/home/x3.hpp>

namespace runir::kr::ps::ext::dl::parser
{
namespace x3 = boost::spirit::x3;

struct ModuleClass;
struct ModuleProgramClass;
struct ModuleRootClass;
struct ModuleProgramRootClass;

using module_type = x3::rule<ModuleClass, ast::Module>;
using module_program_type = x3::rule<ModuleProgramClass, ast::ModuleProgram>;
using module_root_type = x3::rule<ModuleRootClass, ast::Module>;
using module_program_root_type = x3::rule<ModuleProgramRootClass, ast::ModuleProgram>;

BOOST_SPIRIT_DECLARE(module_type, module_program_type, module_root_type, module_program_root_type)

module_type const& module_parser();
module_program_type const& module_program_parser();
module_root_type const& module_root_parser();
module_program_root_type const& module_program_root_parser();

}  // namespace runir::kr::ps::ext::dl::parser

#endif
