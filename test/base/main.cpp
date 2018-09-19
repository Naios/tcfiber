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

#include <cassert>
#include "Async.h"
#include "AsyncCreatureAI.h"
#include "Await.h"
#include "FiberPool.h"
#include "Future.h"

using namespace Trinity;

struct TestAI : public AsyncCreatureAI
{
    void Reset() override
    {
        await OnEnterCombat();

        for (;;)
        {
            await Wait(3s, 6s);

            while ((await CastSpell(2336)) != SpellCastResult::Ok)
            {
                await Wait(2s);
            }
        }
    }
};

static void TestResumeDestroy()
{
    FiberPool pool;
    {
        Future<> enter;
        auto fiber = pool.Spawn([&] {
            TestAI ai;
            await std::move(enter);

            int i = 0;
        });

        fiber->Resume();

        auto promise = enter.GetPromise();
        promise.Resolve();

        assert(fiber->Is(Fiber::State::Finished));
    }

    {
        auto fiber = pool.Spawn([] {
            // ...
            ThisFiber()->Suspend();
        });

        fiber->Resume();

        assert(fiber->Is(Fiber::State::Running));
    }

    {
        auto fiber = pool.Spawn([] {
            // ...
            ThisFiber()->Suspend();
        });

        fiber->Resume();

        fiber->Cancel();

        assert(fiber->Is(Fiber::State::Canceled));
    }

    {
        auto fiber = pool.Spawn([] {
            // ...
            ThisFiber()->Suspend();
        });

        fiber->Resume();

        fiber->Resume();

        fiber->Cancel();

        assert(fiber->Is(Fiber::State::Finished));
    }
}

static void TestAsync()
{
    FiberPool pool;
    {
        Future<> fut;
        auto p = fut.GetPromise();

        auto ptr = pool.Spawn([fut = std::move(fut)]() mutable {
            // ...
            ThisFiber()->Suspend();

            int o1 = await Async([]() mutable {
                // ...
                return 7;
            });

            int o2 = await Async([fut = std::move(fut)]() mutable {
                // ...

                await std::move(fut);

                return 0;
            });
        });

        ptr->Resume();

        p.Resolve();

        ptr->Resume();

        ptr->Cancel();
    }
}

void TestPointer()
{
    FiberPool pool;

    auto fiber = pool.Spawn([] {
        // ...
    });

    WeakFiberPtr ptr;

    ptr = WeakFiberPtr(fiber.Get());
    assert(ptr);
}

int main(int, char**)
{
    TestResumeDestroy();
    TestAsync();
    TestPointer();
}
