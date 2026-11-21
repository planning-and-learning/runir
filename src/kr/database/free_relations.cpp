/*
 * Copyright (C) 2025-2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// The repository of free relations. Compiled on its own, outside the unity batches (src/database/CMakeLists.txt).

#include <yggdrasil/formalism/free_relations.hpp>
#include <yggdrasil/formalism/repository_instantiations.hpp>

// The repository of free relations (yggdrasil/formalism/free_relations.hpp), which the database holds: yggdrasil has no
// library of its own to instantiate it in.
YGG_FORMALISM_REPOSITORY(, ygg::FreeRelationRepository, 2, 1)
YGG_FORMALISM_REPOSITORY_SCHEMA(, ygg::FreeRelationRepository)
