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

template <typename... T>
void RandomEvent(T&&...)
{
}

class MyAI : public AsyncCreatureAI
{
  public:
    void Reset() override
    {
        await OnEnterCombat();

        Async([this] {
            await OnDespawn();

            await this->CastSpell(47474);
        });

        Async([this] {
            while (auto damage = await OnDamageReceived())
            {
                // Do something
            }
        });

        while (true)
        {
            RandomEvent(
                [this] {
                    await Wait(10s, 15s);
                    await CastSpell(28373);
                },
                [this] {
                    await Wait(8s, 13s);

                    while ((await CastSpell(3746)) != SpellCastResult::Ok)
                        ;
                });
        }

        //.Then([] {
        //    // ...
        //})
        //.Then([] {
        //    // ...
        //});

        auto despawn = Async([this] {
            // ...
            await OnDespawn();
        });

        auto who = await OnEnterCombat();
    }
};

int main(int, char**)
{
    return 0;
}
