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

#ifndef TRINITY_ASYNC_STACK_REFERENCE_HPP_DEFINED
#define TRINITY_ASYNC_STACK_REFERENCE_HPP_DEFINED

#include <cassert>
#include <type_traits>
#include <utility>

namespace Trinity {
/// A helper class for tracking two unique objects have 1:1 relationship.
///
/// \attention This class is threadunsafe and may only be used
///            from the same thread!
template <typename Child, typename Referenced>
class StackReference
{
    friend StackReference<Referenced, Child>;

    Referenced* ref_ = nullptr;

  public:
    constexpr StackReference() = default;
    explicit constexpr StackReference(Referenced* ref) noexcept : ref_(ref)
    {
        assert(ref_->ref_ == nullptr);
        ref_->ref_ = static_cast<Child*>(this);
    }
    ~StackReference() noexcept { Unlink(); }

    explicit constexpr StackReference(StackReference const&) = delete;
    explicit constexpr StackReference(StackReference&& right) noexcept
        : ref_(std::exchange(right.ref_, nullptr))
    {
        if (ref_)
        {
            ref_->ref_ = static_cast<Child*>(this);
        }
    }

    constexpr StackReference& operator=(StackReference const&) = delete;
    constexpr StackReference& operator=(StackReference&& right) noexcept
    {
        Unlink();
        if ((ref_ = std::exchange(right.ref_, nullptr)))
        {
            ref_->ref_ = static_cast<Child*>(this);
        }
        return *this;
    }

  protected:
    constexpr void Unlink() noexcept
    {
        if (ref_)
        {
            ref_->ref_ = nullptr;
        }
    }

    constexpr bool HasRef() const noexcept { return bool(ref_); }

    constexpr Referenced* GetRef() noexcept
    {
        assert(HasRef());
        return ref_;
    }
    constexpr Referenced const* GetRef() const noexcept
    {
        assert(HasRef());
        return ref_;
    }
};
} // namespace Trinity

#endif // TRINITY_ASYNC_STACK_REFERENCE_HPP_DEFINED
