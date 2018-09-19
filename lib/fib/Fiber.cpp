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

#include "Fiber.h"
#include <cassert>
#include <utility>
#include "FiberPool.h"

namespace Trinity {
static thread_local FiberPtr current;

Fiber* ThisFiber()
{
    assert(current &&
           "Tried to get the current Fiber without being inside a Fiber!");
    return current.Get();
}

static bool IsDead(Fiber::State state) noexcept
{
    return (state == Fiber::State::Finished) ||
           (state == Fiber::State::Canceled);
}

void Fiber::Emplace(boost::context::fiber&& fiber)
{
    fiber_ = std::move(fiber);
}

void Fiber::SetRunning()
{
    assert(Is(State::NotStarted));
    state_ = State::Running;
}

boost::context::fiber Fiber::Finalize(Fiber* fiber)
{
    assert(fiber->Is(State::Running));
    fiber->state_ = State::Finished;
    auto context = std::move(fiber->fiber_);
    assert(IsDead(fiber->state_));

    // This destroys the Fibe eventually and causes
    // the pointer to be invalidated
    current = std::exchange(fiber->previous_, nullptr);
    return context;
}

Fiber::~Fiber()
{
    assert(!previous_);
}

bool Fiber::Is(State state) const noexcept
{
    return state_ == state;
}

void Fiber::Resume()
{
    WeakFiberPtr guard(this);
    (void)guard;
    assert(!IsDead(state_));
    assert(!previous_);
    previous_ = std::exchange(current, this);
    fiber_ = std::move(fiber_).resume();
}

void Fiber::Suspend()
{
    assert(!IsDead(state_));
    current = std::exchange(previous_, nullptr);
    fiber_ = std::move(fiber_).resume();
}

void Fiber::Cancel()
{
    if (Is(State::Running))
    {
        state_ = State::Canceled;
        fiber_ = boost::context::fiber{};
    }
}

void IncreaseRefCounter(Fiber* fiber, StrongWeakType type) noexcept
{
    if (type == StrongWeakType::Strong)
    {
        assert(fiber->strong_count_ > 0);
        ++(fiber->strong_count_);
    }
    else
    {
        assert(type == StrongWeakType::Weak);
        ++(fiber->weak_count_);
    }
}

void DecreaseRefCounter(Fiber* fiber, StrongWeakType type) noexcept
{
    if (type == StrongWeakType::Strong)
    {
        assert(fiber->strong_count_ > 0);
        --(fiber->strong_count_);

        if (fiber->strong_count_ == 0)
        {
            // Cancel the running fiber when there is
            // no strong reference anymore
            fiber->Cancel();
        }
    }
    else
    {
        assert(type == StrongWeakType::Weak);
        assert(fiber->weak_count_ > 0);
        --(fiber->weak_count_);
    }

    if ((fiber->strong_count_ + fiber->weak_count_) == 0)
    {
        fiber->pool_.Recycle(fiber);
    }
}
} // namespace Trinity
