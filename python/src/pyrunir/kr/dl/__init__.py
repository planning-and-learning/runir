# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""Description logic constructors, repositories and evaluation."""

from pyrunir._pyrunir.kr.dl import (
    ArgumentBoolean as ArgumentBoolean,
    ArgumentBooleanData as ArgumentBooleanData,
    ArgumentBooleanIndex as ArgumentBooleanIndex,
    ArgumentConcept as ArgumentConcept,
    ArgumentConceptData as ArgumentConceptData,
    ArgumentConceptIndex as ArgumentConceptIndex,
    ArgumentNumerical as ArgumentNumerical,
    ArgumentNumericalData as ArgumentNumericalData,
    ArgumentNumericalIndex as ArgumentNumericalIndex,
    ArgumentRole as ArgumentRole,
    ArgumentRoleData as ArgumentRoleData,
    ArgumentRoleIndex as ArgumentRoleIndex,
    BooleanArgumentIdentifier as BooleanArgumentIdentifier,
    ConceptArgumentIdentifier as ConceptArgumentIdentifier,
    ConceptRegisterIdentifier as ConceptRegisterIdentifier,
    ConstructorRepository as ConstructorRepository,
    NumericalArgumentIdentifier as NumericalArgumentIdentifier,
    RegisterConcept as RegisterConcept,
    RegisterConceptData as RegisterConceptData,
    RegisterConceptIndex as RegisterConceptIndex,
    RegisterRole as RegisterRole,
    RegisterRoleData as RegisterRoleData,
    RegisterRoleIndex as RegisterRoleIndex,
    RoleArgumentIdentifier as RoleArgumentIdentifier,
    RoleRegisterIdentifier as RoleRegisterIdentifier,
)
from pyrunir.kr.dl import (
    cnf_grammar as cnf_grammar,
    grammar as grammar,
    semantics as semantics,
)
