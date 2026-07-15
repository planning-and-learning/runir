#include "pyrunir/kr/uns/module.hpp"

#include <runir/kr/uns/classifier_index.hpp>
#include <runir/kr/uns/feature_index.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::uns
{

void bind_indices(nb::module_& m)
{
    ygg::bind_index<ygg::Index<runir::kr::uns::Feature>>(m, "FeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::uns::ClassifierLiteral>>(m, "ClassifierLiteralIndex");
    ygg::bind_index<ygg::Index<runir::kr::uns::ClassifierClause>>(m, "ClassifierClauseIndex");
    ygg::bind_index<ygg::Index<runir::kr::uns::Classifier>>(m, "ClassifierIndex");
}

}  // namespace runir::kr::uns
