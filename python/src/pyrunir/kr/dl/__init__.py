# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""Description logic constructors, repositories and evaluation."""

from pyrunir._pyrunir.kr.dl import (
    BooleanArgument as BooleanArgument,
    BooleanArgumentData as BooleanArgumentData,
    BooleanArgumentIdentifier as BooleanArgumentIdentifier,
    BooleanArgumentIndex as BooleanArgumentIndex,
    ConceptArgument as ConceptArgument,
    ConceptArgumentData as ConceptArgumentData,
    ConceptArgumentIdentifier as ConceptArgumentIdentifier,
    ConceptArgumentIndex as ConceptArgumentIndex,
    ConceptRegister as ConceptRegister,
    ConceptRegisterData as ConceptRegisterData,
    ConceptRegisterIdentifier as ConceptRegisterIdentifier,
    ConceptRegisterIndex as ConceptRegisterIndex,
    ConstructorRepository as ConstructorRepository,
    ConstructorRepositoryFactory as ConstructorRepositoryFactory,
    NumericalArgument as NumericalArgument,
    NumericalArgumentData as NumericalArgumentData,
    NumericalArgumentIdentifier as NumericalArgumentIdentifier,
    NumericalArgumentIndex as NumericalArgumentIndex,
    RoleArgument as RoleArgument,
    RoleArgumentData as RoleArgumentData,
    RoleArgumentIdentifier as RoleArgumentIdentifier,
    RoleArgumentIndex as RoleArgumentIndex,
    RoleRegister as RoleRegister,
    RoleRegisterData as RoleRegisterData,
    RoleRegisterIdentifier as RoleRegisterIdentifier,
    RoleRegisterIndex as RoleRegisterIndex,
)
from pyrunir.kr.dl import (
    cnf_grammar as cnf_grammar,
    grammar as grammar,
    semantics as semantics,
)
