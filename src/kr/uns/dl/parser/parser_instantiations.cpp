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

classifier_root_type const& classifier_root_parser() { return classifier_root; }

}  // namespace runir::kr::uns::dl::parser
