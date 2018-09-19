/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINITY_ASYNC_AWAITABLE_HPP_DEFINED
#define TRINITY_ASYNC_AWAITABLE_HPP_DEFINED

#include <cassert>
#include <type_traits>
#include <utility>
#include "Fiber.h"

namespace Trinity {
namespace Detail {
struct Awaiter;
}

template <typename T>
struct AwaitableTrait
{
    static_assert(!std::is_same<T, T>::value,
                  "The AwaitableTrait is not implemented for this class!");

    /// Returns true when the Awaitable is resolved already
    static bool IsReady(T const& /*awaitable*/) { return false; }

    /// Suspends the current fiber until the result is available
    static void Await(T& /*awaitable*/) {}

    /// Returns the resolved result values of the given Awaitable
    static void Unpack(T&& /*awaitable*/) {}
};
} // namespace Trinity

#endif // TRINITY_ASYNC_AWAITABLE_HPP_DEFINED
