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

#ifndef TRINITY_ASYNC_IMPL_HPP_DEFINED
#define TRINITY_ASYNC_IMPL_HPP_DEFINED

#include <tuple>
#include <type_traits>
#include <utility>
#include "Fiber.h"
#include "FiberPool.h"
#include "Future.h"

namespace Trinity {
namespace Detail {
template <typename T>
struct AsyncTrait
{
    using FutureType = Future<T>;

    template <typename Callable>
    static void Resolve(Promise<T>& promise, Callable&& callable)
    {
        auto res = std::forward<Callable>(callable)();
        promise.Resolve(std::move(res));
    }
};
template <>
struct AsyncTrait<void>
{
    using FutureType = Future<>;

    template <typename Callable>
    static void Resolve(Promise<>& promise, Callable&& callable)
    {
        std::forward<Callable>(callable)();
        promise.Resolve();
    }
};

// Multiple Arguments aren't supported by now
// template <typename... Args>
// struct AsyncTrait<std::tuple<Args...>>;

struct AsyncImpl
{
    template <typename Callable>
    static auto Async(Callable&& callable)
    {
        using Trait = AsyncTrait<decltype(std::declval<Callable>()())>;
        typename Trait::FutureType future;
        FiberPtr fiber = ThisFiber()->Pool().Spawn(
            [callable = std::forward<Callable>(callable),
             promise = future.GetPromise()]() mutable {
                Trait::Resolve(promise, std::move(callable));
            });

        // It is important here to create a copy of the FiberPtr since
        // the spawned Fiber could return instantly to resolve the future
        // which would cause a pointer invalidation.
        // TODO Remove the copy
        Fiber* current = fiber.Get();
        future.SetResolvedFrom(std::move(fiber));
        current->Resume();
        return std::move(future);
    }
};
} // namespace Detail
} // namespace Trinity

#endif // TRINITY_ASYNC_TRAIT_HPP_DEFINED
