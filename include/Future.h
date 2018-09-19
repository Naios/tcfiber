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

#ifndef TRINITY_ASYNC_FUTURE_HPP_DEFINED
#define TRINITY_ASYNC_FUTURE_HPP_DEFINED

#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Awaitable.h"
#include "StackReference.h"

namespace Trinity {
template <typename...>
class Future;
namespace Detail {
struct AsyncImpl;
struct FutureFactory;
struct FutureAwaitableTraitBase;
struct FutureReadyInitTag
{
};
} // namespace Detail

/// The Promise represents a Future resolver with an extremely low memory
/// footprint, that can be used to resolve a Future to a later timepoint.
///
/// \attention The Promise is required to resolve the future before destruction!
template <typename... Args>
class Promise : public StackReference<Promise<Args...>, Future<Args...>>
{
    friend class Future<Args...>;

    explicit constexpr Promise(Future<Args...>* future) noexcept
        : StackReference<Promise<Args...>, Future<Args...>>(future)
    {
    }

  public:
    ~Promise() noexcept
    {
        assert((!this->HasRef() || this->GetRef()->IsReady()) &&
               "The promise is destroyed before the future was resolved!");
    }
    constexpr Promise(Promise const&) = default;
    constexpr Promise(Promise&&) = default;
    constexpr Promise& operator=(Promise const&) = default;
    constexpr Promise& operator=(Promise&&) = default;

    /// Returns true when the Promise result isn't needed anymore
    bool IsCanceled() const noexcept
    {
        return !this->HasRef() || this->GteRef()->IsCanceled();
    }

    /// Resolves the connected Future with the given arguments
    void Resolve(Args... args)
    {
        if (this->HasRef())
        {
            // It is valid that the future is destroyed before it was
            // resolved through the promise in case the execution of
            // the parent Fiber isn't required anymore.
            this->GetRef()->ResolveResult(std::forward<Args>(args)...);
        }
    }
};

/// Represents a an arbitrary count of result values which may be
/// resolved in the future.
///
/// Use \see await to suspend the currently executed Fiber until
/// this future is ready.
template <typename... Args>
class Future : public StackReference<Future<Args...>, Promise<Args...>>
{
    friend Detail::AsyncImpl;
    friend Detail::FutureFactory;
    friend Detail::FutureAwaitableTraitBase;
    friend Promise<Args...>;
    friend AwaitableTrait<Future<Args...>>;

    /// The fiber which waits for the completion of this result
    WeakFiberPtr waiting_fiber_;

    /// A strong reference to the resolving fiber which causes the
    /// resolving fiber to be destroyed automatically when this Future
    /// is dropped and the result isn't needed anymore.
    FiberPtr resolver_;

    /// Caches the result until it was returned
    boost::optional<std::tuple<Args...>> result_;

    explicit constexpr Future(Detail::FutureReadyInitTag, Args... args) noexcept
        : result_(std::forward<Args>(args)...)
    {
    }

  public:
    explicit constexpr Future() noexcept = default;

    ~Future() noexcept = default;
    constexpr Future(Future const&) = default;
    constexpr Future(Future&&) = default;
    constexpr Future& operator=(Future const&) = default;
    constexpr Future& operator=(Future&&) = default;

    /// Returns true when the Future was resolved
    bool IsReady() const noexcept { return bool(result_); }
    /// Returns true when the Future result isn't needed anymore
    bool IsCanceled() const noexcept { return bool(result_); }

    /// Returns a Promise which is connected to this Future,
    /// that can be used to resolve the Future later.
    Promise<Args...> GetPromise() noexcept
    {
        assert(!this->HasRef() && "A promise was retrieved already!");
        assert(!IsReady() && "Don't create a promise for a resolved future!");
        return Promise<Args...>(this);
    }

    // template<typename T>
    // Future& Then(T&& callable) { return *this;}

  private:
    void ResolveResult(Args... args)
    {
        if (!IsCanceled())
        {
            assert(!IsReady() && "The future is resolved already!");
            result_.emplace(std::forward<Args>(args)...);
            assert(IsReady());
            resolver_ = nullptr;

            if (waiting_fiber_)
            {
                assert(!(waiting_fiber_->Is(Fiber::State::Finished) ||
                         waiting_fiber_->Is(Fiber::State::Canceled)));

                waiting_fiber_->Resume();
            }
        }
    }
    void SetResolvedFrom(FiberPtr fiber)
    {
        assert(!resolver_ && "This future has a resolver registered already!");
        resolver_ = std::move(fiber);
    }
};

namespace Detail {
struct FutureFactory
{
    template <typename... Args>
    static constexpr auto CreateFrom(Args&&... args)
    {
        return Future<std::decay_t<Args>...>(FutureReadyInitTag{},
                                             std::forward<Args>(args)...);
    }
};
} // namespace Detail

/// Creates a resolved ready Future from the given arguments.
template <typename... Args>
constexpr Future<std::decay_t<Args>...> MakeReadyFuture(Args&&... args)
{
    return Detail::FutureFactory::CreateFrom(std::forward<Args>(args)...);
}

namespace Detail {
struct FutureAwaitableTraitBase
{
    template <typename T>
    static bool IsReady(T&& future)
    {
        return future.IsReady(); //
    }

    template <typename T>
    static void Await(T&& future)
    {
        Fiber* const fiber = ThisFiber();
        assert(!future.waiting_fiber_ &&
               "await was used on this Future already!");

        future.waiting_fiber_ = WeakFiberPtr(fiber);
        fiber->Suspend();
    }
};
} // namespace Detail

template <>
struct AwaitableTrait<Future<>> : Detail::FutureAwaitableTraitBase
{
    static void Unpack(Future<>&& /*awaitable*/)
    {
        // Nothing to do here
    }
};
template <typename Arg>
struct AwaitableTrait<Future<Arg>> : Detail::FutureAwaitableTraitBase
{
    static auto Unpack(Future<Arg>&& awaitable)
    {
        return std::move(std::get<0>(*awaitable.result_));
    }
};
template <typename FirstArg, typename SecondArg, typename... Args>
struct AwaitableTrait<Future<FirstArg, SecondArg, Args...>>
    : Detail::FutureAwaitableTraitBase
{
    static auto Unpack(Future<FirstArg, SecondArg, Args...>&& awaitable)
    {
        return std::move(*awaitable.result_);
    }
};
} // namespace Trinity

#endif // TRINITY_ASYNC_FUTURE_HPP_DEFINED
