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

#ifndef TRINITY_ASYNC_ASYNC_HPP_DEFINED
#define TRINITY_ASYNC_ASYNC_HPP_DEFINED

#include <utility>
#include "AsyncImpl.h"

namespace Trinity {
template <typename Callable, typename Trait = Detail::AsyncTrait<
                                 decltype(std::declval<Callable>()())>>
auto Async(Callable&& callable) -> typename Trait::FutureType
{
    return Detail::AsyncImpl::Async(std::forward<Callable>(callable));
}
} // namespace Trinity

#endif // TRINITY_ASYNC_ASYNC_HPP_DEFINED
