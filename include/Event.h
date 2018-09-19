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

#ifndef TRINITY_ASYNC_EVENT_HPP_DEFINED
#define TRINITY_ASYNC_EVENT_HPP_DEFINED

#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Awaitable.h"
#include "StackReference.h"

namespace Trinity {
template <typename... Args>
class Event : public Awaitable
{
    friend Sink<Args...>;
    friend AwaitableTrait<Channel<Args...>>;

    /// A strong reference to the resolving fiber which causes the
    /// resolving fiber to be destroyed automatically when this Channel
    /// is dropped and the result isn't needed anymore.
    FiberPtr resolver_;

    /// Caches the result until it was returned
    boost::optional<std::tuple<Args...>> result_;

    explicit constexpr Channel(Detail::FutureReadyInitTag,
                               Args... args) noexcept
        : result_(std::forward<Args>(args)...)
    {
    }

  public:
    explicit constexpr Channel() noexcept = default;

    ~Channel() noexcept = default;
    constexpr Channel(Channel const&) noexcept = default;
    constexpr Channel(Channel&&) noexcept = default;
    constexpr Channel& operator=(Channel const&) noexcept = default;
    constexpr Channel& operator=(Channel&&) noexcept = default;

    /// Returns true when the Channel was resolved
    bool IsReady() const noexcept { return bool(result_); }
    /// Returns true when the Channel result isn't needed anymore
    bool IsCanceled() const noexcept { return bool(result_); }

    /// Returns a Sink which is connected to this Channel,
    /// that can be used to resolve the Channel later.
    Sink<Args...> GetPromise() noexcept
    {
        assert(!this->HasRef() && "A promise was retrieved already!");
        assert(!IsReady() && "Don't create a promise for a resolved future!");
        return Sink<Args...>(this);
    }

  private:
    void ResolveResult(Args... args)
    {
        if (!IsCanceled())
        {
            assert(!IsReady() && "The future is resolved already!");
            result_.emplace(std::forward<Args>(args)...);
            assert(IsReady());
            resolver_ = nullptr;
            AwaitResume();
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
        return Channel<std::decay_t<Args>...>(FutureReadyInitTag{},
                                              std::forward<Args>(args)...);
    }
};
} // namespace Detail

/// Creates a resolved ready Channel from the given arguments.
template <typename... Args>
constexpr Channel<std::decay_t<Args>...> MakeReadyFuture(Args&&... args)
{
    return Detail::FutureFactory::CreateFrom(std::forward<Args>(args)...);
}

template <>
struct AwaitableTrait<Channel<>>
{
    static bool IsReady(Channel<> const& awaitable)
    {
        return awaitable.IsReady(); //
    }

    static void Unpack(Channel<>&& /*awaitable*/)
    {
        // Nothing to do here
    }
};
template <typename Arg>
struct AwaitableTrait<Channel<Arg>>
{
    static bool IsReady(Channel<Arg> const& awaitable)
    {
        return awaitable.IsReady(); //
    }

    static auto Unpack(Channel<Arg>&& awaitable)
    {
        return std::move(std::get<0>(*awaitable.result_));
    }
};
template <typename FirstArg, typename SecondArg, typename... Args>
struct AwaitableTrait<Channel<FirstArg, SecondArg, Args...>>
{
    static bool IsReady(Channel<FirstArg, SecondArg, Args...> const& awaitable)
    {
        return awaitable.IsReady();
    }

    static auto Unpack(Channel<FirstArg, SecondArg, Args...>&& awaitable)
    {
        return std::move(*awaitable.result_);
    }
};
} // namespace Trinity

#endif // TRINITY_ASYNC_EVENT_HPP_DEFINED
