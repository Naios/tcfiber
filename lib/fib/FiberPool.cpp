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

#include "FiberPool.h"
#include <cassert>
#include <boost/context/protected_fixedsize_stack.hpp>
#include <boost/context/stack_traits.hpp>

#ifndef TC_FIBER_PROTECT
#ifndef NDEBUG
#define TC_FIBER_PROTECT
#endif
#endif

namespace Trinity {
static std::size_t PageSize() noexcept
{
    return boost::context::stack_traits::page_size();
}

static std::size_t ProtectionPageSize() noexcept
{
#ifdef TC_FIBER_PROTECT
    return PageSize();
#else
    return 0;
#endif
}

static std::size_t StackSize() noexcept
{
    // The size of 10 kb is required on Windows so the unwind exceptions work
    // there. Strange behaviour was seen when using less stack space.
    //
    // Additionally when defining TC_FIBER_PROTECT we protect the memory
    // page on bottom of the stack to prevent memory corruption through
    // silent stack overflows.
    return 1024 * 12 + ProtectionPageSize();
}

static constexpr std::size_t MaxAllocatedChunks() noexcept
{
#ifdef TC_FIBER_PROTECT
    // We only allocate one fiber per block since we are using protected
    // stacks with guard pages when TC_FIBER_PROTECT is defined.
    return 1;
#else
    return 0;
#endif
}

static constexpr std::size_t DefaultAllocatedChunks() noexcept
{
#ifdef TC_FIBER_PROTECT
    return MaxAllocatedChunks();
#else
    return 4;
#endif
}

char* FiberPool::PoolAllocator::malloc(size_type bytes)
{
#ifdef TC_FIBER_PROTECT
    // Allocate multiple pages of memory for the Fiber stack,
    // where the top page is a protected page that causes a segfault on write,
    // in order to protect against stack overflows.
    boost::context::protected_fixedsize_stack alloc(bytes);
    auto const stack = alloc.allocate();
    return static_cast<char*>(stack.sp) - stack.size + PageSize();
#else
    return static_cast<char*>(std::malloc(bytes));
#endif
}

void FiberPool::PoolAllocator::free(char* block)
{
#ifdef TC_FIBER_PROTECT
    boost::context::protected_fixedsize_stack alloc(0);
    boost::context::stack_context context{0, block - PageSize()};
    alloc.deallocate(context);
#else
    std::free(block);
#endif
}

boost::context::stack_context FiberPool::FiberAllocator::allocate()
{
    // Unreachable
    assert(false);
    return {0, nullptr};
}

void FiberPool::FiberAllocator::deallocate(boost::context::stack_context&) {}

FiberPool::FiberPool()
    : pool_(StackSize(), DefaultAllocatedChunks(), MaxAllocatedChunks())
#ifndef NDEBUG
      ,
      allocated_(0)
#endif
{
}

FiberPool::~FiberPool()
{
    assert(allocated_ == 0 &&
           "The FiberPool is being destroyed with allocated Fibers left!");
}

template <typename T>
T* AllocateOnStack(void*& sp, std::size_t& size)
{
    void* const storage = reinterpret_cast<void*>(
        (reinterpret_cast<uintptr_t>(sp) - static_cast<uintptr_t>(sizeof(T))) &
        ~static_cast<uintptr_t>(0xff));

    static_assert(sizeof(Fiber) <= 64, "");
    assert(storage < static_cast<char*>(sp) + size);

    sp = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(storage) -
                                 static_cast<uintptr_t>(64));
    void* const stack_bottom = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(sp) - static_cast<uintptr_t>(size));

    size = reinterpret_cast<uintptr_t>(sp) -
           reinterpret_cast<uintptr_t>(stack_bottom);

    return static_cast<T*>(storage);
}

FiberPool::FiberAllocation FiberPool::AllocateFiber()
{
    auto const size = pool_.get_requested_size();
    void* const stack = pool_.malloc();

#ifndef NDEBUG
    ++allocated_;
#endif

    boost::context::stack_context context{size,
                                          static_cast<char*>(stack) + size};

    // Write the Fiber data on the bottom of the stack
    Fiber* fiber = AllocateOnStack<Fiber>(context.sp, context.size);
    new (fiber) Fiber(*this, stack);

    boost::context::preallocated pre(context.sp, context.size, context);

    return FiberAllocation{FiberPtr{fiber, false}, std::move(pre)};
}

void FiberPool::Recycle(Fiber* fiber) noexcept
{
    fiber->~Fiber();
    pool_.free(fiber->stack_);

#ifndef NDEBUG
    --allocated_;
#endif
}
} // namespace Trinity
