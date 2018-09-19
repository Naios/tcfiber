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

#ifndef TRINITY_FIBER_POOL_HPP_DEFINED
#define TRINITY_FIBER_POOL_HPP_DEFINED

#include <cstddef>
#include <tuple>
#include <boost/context/fiber.hpp>
#include <boost/pool/pool.hpp>
#include "Fiber.h"

namespace Trinity {
/// Represents the origin of a Fiber.
/// The FiberPool is responsible for recyling the Fibers after usage
/// in order to improve the speed and memory footprint of spawned Fibers.
///
/// \attention The FiberPool is thread unsafe and may not be passed
///            or used to from multiple threads!
class FiberPool
{
    friend void DecreaseRefCounter(Fiber*, StrongWeakType) noexcept;

    struct PoolAllocator
    {
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        static char* malloc(size_type bytes);
        static void free(char* block);
    };
    boost::pool<PoolAllocator> pool_;

#ifndef NDEBUG
    std::size_t allocated_ = 0;
#endif

    struct FiberAllocator
    {
        boost::context::stack_context allocate();
        void deallocate(boost::context::stack_context& context);
    };

  public:
    explicit FiberPool();
    ~FiberPool();
    FiberPool(FiberPool const&) = delete;
    FiberPool(FiberPool&&) = delete;
    FiberPool& operator=(FiberPool const&) = delete;
    FiberPool& operator=(FiberPool&&) = delete;

    /// Creates a fiber which invokes the given callable,
    /// that must accept the signature of `void()`.
    ///
    /// \attention By default the Fiber isn't executed automatically
    ///            and thus must be invoked through Fiber::Resume.
    template <typename Callable>
    FiberPtr Spawn(Callable&& callable)
    {
        auto alloc = AllocateFiber();
        alloc.fiber->Emplace(boost::context::fiber(
            std::allocator_arg, alloc.pre, FiberAllocator{},
            [callable = std::forward<Callable>(callable)](
                boost::context::fiber && sink) mutable {
                // The current executed fiber will always be the same
                Fiber* const fiber = ThisFiber();

                // Invoke the user provided callback
                fiber->SetRunning();
                fiber->Emplace(std::move(sink));
                callable();

                // Finish the fiber execution and eventually destroy the fiber
                return Fiber::Finalize(fiber);
            }));

        return std::move(alloc.fiber);
    }

  private:
    struct FiberAllocation
    {
        FiberPtr fiber;
        boost::context::preallocated pre;
        explicit FiberAllocation(FiberPtr&& fiber_,
                                 boost::context::preallocated&& pre_)
            : fiber(std::move(fiber_)), pre(std::move(pre_))
        {
        }
    };
    FiberAllocation AllocateFiber();

    void Recycle(Fiber* fiber) noexcept;
};
} // namespace Trinity

#endif // TRINITY_FIBER_POOL_HPP_DEFINED
