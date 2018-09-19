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

#ifndef TRINITY_ASYNC_CREATURE_AI_HPP_DEFINED
#define TRINITY_ASYNC_CREATURE_AI_HPP_DEFINED

#include <chrono>
#include "Await.h"
#include "Future.h"

using namespace std::chrono_literals;

namespace Trinity {
struct Unit
{
};
struct Creature : public Unit
{
};
struct Player : public Unit
{
};

enum class SpellCastResult
{
    Ok,
    Failed
};

class AsyncCreatureAI
{
  public:
    virtual void Reset() {}

    Future<Unit*> OnEnterCombat()
    {
        return MakeReadyFuture(static_cast<Unit*>(nullptr));
    }
    Future<Unit*> OnDespawn()
    {
        return MakeReadyFuture(static_cast<Unit*>(nullptr));
    }
    Future<uint32_t> OnDamageReceived()
    {
        return MakeReadyFuture<uint32_t>(2000);
    }
    Future<SpellCastResult> CastSpell(unsigned)
    {
        Future<SpellCastResult> future;
        auto promise = future.GetPromise();
        promise.Resolve(SpellCastResult::Ok);
        return future;
    }
    Future<> Wait(std::chrono::milliseconds)
    {
        Future<> future;
        auto promise = future.GetPromise();
        promise.Resolve();
        return future;
    }
    Future<> Wait(std::chrono::milliseconds, std::chrono::milliseconds)
    {
        return MakeReadyFuture();
    }

    template <typename Callable>
    auto Async(Callable&&)
    {
        return 1;
    }
};
} // namespace Trinity

#endif // TRINITY_ASYNC_CREATURE_AI_HPP_DEFINED
