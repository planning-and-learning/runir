/*
 * Copyright (C) 2025-2026 Dominik Drexler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

// The entry points of the serialization: each compiles on its own, as a user includes it alone.
#include "runir/serialization/serialization.hpp"

#include "runir/serialization/types.hpp"

#include <gtest/gtest.h>

namespace
{

TEST(Serialization, EntryHeadersAreSelfContained) { SUCCEED(); }

}  // namespace
