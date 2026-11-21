/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_OPTIMIZATION_DETAILS_JOIN_BLOCK_HPP_
#define RUNIR_KR_DATABASE_OPTIMIZATION_DETAILS_JOIN_BLOCK_HPP_

#include "runir/kr/database/syntax/operator_builder.hpp"
#include "runir/kr/database/syntax/query.hpp"
#include "yggdrasil/semantics/canonicalization.hpp"

#include <algorithm>
#include <concepts>
#include <span>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace runir::kr::database::detail
{
/// A natural join of atoms whose output keeps the required variables: atom_1 ⋈ … ⋈ atom_n.
/// Atoms are planned operands; columns with equal labels are join variables.
struct JoinBlock
{
    urd::Store<urd::sequence<QueryView<>>> atoms;
    /// The output variables as a sorted set.
    urd::Store<urd::sequence<urd::index<ygg::Column>>> required;
};

urd::Store<urd::sequence<urd::index<ygg::Column>>> variables(QueryView<> query);

/// R ⋉ S, written R ⋈ π_common(S) with the existing operators, as Yannakakis' semijoin programs use it.
QueryView<> semijoin(OperatorBuilder& build, QueryView<> left_operand, QueryView<> right_operand);

/// The atoms' variables ordered by label.
urd::Store<urd::sequence<urd::index<ygg::Column>>> variable_order(std::span<const QueryView<>> atoms);
}  // namespace runir::kr::database::detail

#endif
