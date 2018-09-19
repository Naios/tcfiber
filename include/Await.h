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

#ifndef TRINITY_ASYNC_AWAIT_HPP_DEFINED
#define TRINITY_ASYNC_AWAIT_HPP_DEFINED

#include <cassert>
#include <type_traits>
#include "Awaitable.h"
#include "Fiber.h"

namespace Trinity {
namespace Detail {
struct Awaiter
{
    template <typename Awaitable>
    auto operator<<(Awaitable&& awaitable) noexcept(false)
    {
        static_assert(std::is_rvalue_reference<Awaitable&&>::value,
                      "The awaitable must be passed as r-value reference!");

        using Trait = AwaitableTrait<std::decay_t<Awaitable>>;

        if (!Trait::IsReady(awaitable))
        {
            assert(ThisFiber() && "We should always be on a fiber here!");
            assert(!(ThisFiber()->Is(Fiber::State::Finished) ||
                     ThisFiber()->Is(Fiber::State::Canceled)));
            Trait::Await(awaitable);
            assert(Trait::IsReady(awaitable));
        }

        return Trait::Unpack(std::forward<Awaitable>(awaitable));
    }
};

/// Represents a keyword which suspends the current Fiber until the given
/// Awaitable to the right side becomes ready and returns the result values
/// of the Awaitable.
#define await (Trinity::Detail::Awaiter{}) <<
} // namespace Detail
} // namespace Trinity

#endif // TRINITY_ASYNC_AWAIT_HPP_DEFINED
