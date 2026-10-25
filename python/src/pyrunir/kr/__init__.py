# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""Knowledge representation: description logics, policy sketches and unsolvability classifiers."""

from pyrunir._pyrunir.kr import (
    ArityMismatchError as ArityMismatchError,
    DomainContext as DomainContext,
    DuplicateDefinitionError as DuplicateDefinitionError,
    GroundTaskContext as GroundTaskContext,
    InvalidExpressionError as InvalidExpressionError,
    LiftedTaskContext as LiftedTaskContext,
    ParseError as ParseError,
    SemanticError as SemanticError,
    UndefinedSymbolError as UndefinedSymbolError,
)
from pyrunir.kr import (
    dl as dl,
    ps as ps,
    uns as uns,
)
