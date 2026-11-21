#ifndef PYRUNIR_KR_DATABASE_BINDINGS_HPP_
#define PYRUNIR_KR_DATABASE_BINDINGS_HPP_

#include "module.hpp"
#include "runir/kr/database/semantics/relation_view.hpp"
#include "yggdrasil/formalism/free_relations.hpp"
#include "yggdrasil/formalism/relation_builder.hpp"
#include "yggdrasil/python/relations.hpp"

#include <algorithm>
#include <span>
#include <stdexcept>

namespace runir::kr::database
{

using Column = urd::index<ygg::Column>;
using ColumnIndices = std::span<const urd::data<ygg::ColumnLayout>>;
using ScratchRelation = ygg::FreeScratchRelation;

/// Python is a trust boundary: the operations only assert (debug builds) that their relations have the plan's columns,
/// so the bindings check it, throwing ValueError.
inline void require_columns(std::span<const urd::data<ygg::ColumnLayout>> actual, std::span<const urd::data<ygg::ColumnLayout>> expected)
{
    if (!std::ranges::equal(actual, expected))
        throw std::invalid_argument("Relational operation: columns do not match the plan.");
}

void bind_datalog(nb::module_& m);
void bind_distance(nb::module_& m);
void bind_evaluation(nb::module_& m);
void bind_optimization(nb::module_& m);
void bind_queries(nb::module_& m);

}  // namespace runir::kr::database

#endif
