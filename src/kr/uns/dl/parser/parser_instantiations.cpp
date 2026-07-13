#include "runir/kr/uns/dl/parser/parser_def.hpp"

namespace runir::kr::uns::dl::parser
{

BOOST_SPIRIT_INSTANTIATE(boolean_feature_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(positive_literal_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(negative_literal_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(literal_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(conjunction_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(disjunction_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(classifier_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(classifier_root_type, runir::kr::parser::Iterator, runir::kr::parser::Context)

boolean_feature_type const& boolean_feature_parser() { return boolean_feature; }
positive_literal_type const& positive_literal_parser() { return positive_literal; }
negative_literal_type const& negative_literal_parser() { return negative_literal; }
literal_type const& literal_parser() { return literal; }
conjunction_type const& conjunction_parser() { return conjunction; }
disjunction_type const& disjunction_parser() { return disjunction; }
classifier_type const& classifier_parser() { return classifier; }
classifier_root_type const& classifier_root_parser() { return classifier_root; }

}  // namespace runir::kr::uns::dl::parser
