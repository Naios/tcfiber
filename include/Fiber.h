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

#ifndef TRINITY_ASYNC_FIBER_HPP_DEFINED
#define TRINITY_ASYNC_FIBER_HPP_DEFINED

#include <cstddef>
#include <boost/context/fiber.hpp>
#include "IntrusivePtr.h"

namespace Trinity {
class Fiber;
class FiberPool;

/// A managed pointer to a Fiber that which causes the Fiber to stay
/// alive until all instances of the pointer are destroyed.
///
/// \attention The FiberPtr is thread unsafe and may not be passed
///            to other threads!
using FiberPtr = IntrusivePtr<Fiber, StrongWeakType, StrongWeakType::Strong>;

/// A weakly referenced counterpart to FiberPtr
///
/// See FiberPtr for details.
using WeakFiberPtr = IntrusivePtr<Fiber, StrongWeakType, StrongWeakType::Weak>;

/// Represents a suspendable control flow which may be resumed at any time.
/// The Fiber can be spawned from a FiberPool, which also effectively recycles
/// the Fiber on re-usage.
///
/// \attention The Fiber is thread unsafe and may not be passed
///            to multiple threads!
class Fiber
{
  public:
    enum class State
    {
        NotStarted,
        Running,
        Finished,
        Canceled
    };

  private:
    friend FiberPool;
    State state_ = State::NotStarted;
    std::size_t strong_count_ = 1;
    std::size_t weak_count_ = 0;
    FiberPtr previous_;
    FiberPool& pool_;
    void* const stack_;
    boost::context::fiber fiber_;

    explicit Fiber(FiberPool& pool, void* stack) noexcept
        : pool_(pool), stack_(stack)
    {
    }

    void Emplace(boost::context::fiber&& fiber);
    void SetRunning();
    static boost::context::fiber Finalize(Fiber* fiber);

  public:
    ~Fiber();
    constexpr Fiber(Fiber const&) noexcept = delete;
    constexpr Fiber(Fiber&&) noexcept = delete;
    constexpr Fiber& operator=(Fiber const&) noexcept = delete;
    constexpr Fiber& operator=(Fiber&&) noexcept = delete;

    /// Returns true when the fiber is in the given state
    bool Is(State state) const noexcept;

    /// Resumes the execution of this Fiber and continues where the
    /// Fiber was suspended last.
    void Resume();

    /// Suspends the Fiber and resumes the execution control back
    /// to the calling thread or Fiber.
    ///
    /// \attention This may only be called from within this Fiber!
    void Suspend();

    /// Destroys the Fiber and unwinds the remaining callstack.
    /// This method is safe to use even after the Fiber has finished.
    void Cancel();

    /// Returns the FiberPool the Fiber is originating from
    FiberPool& Pool() noexcept { return pool_; }

    friend void IncreaseRefCounter(Fiber* fiber, StrongWeakType type) noexcept;
    friend void DecreaseRefCounter(Fiber* fiber, StrongWeakType type) noexcept;
};

/// Returns a managed pointer to the currently executed Fiber.
///
/// \warning   This function requires to be invoked from within a fiber,
///            otherwise an assertion is triggered!
///
/// \attention This function is threadsafe, and may be called concurrently
///            from multiple threads.
Fiber* ThisFiber();
} // namespace Trinity

#endif // TRINITY_ASYNC_FIBER_HPP_DEFINED
