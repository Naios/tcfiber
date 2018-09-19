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

#ifndef TRINITY_ASYNC_STACK_LIST_REFERENCE_HPP_DEFINED
#define TRINITY_ASYNC_STACK_LIST_REFERENCE_HPP_DEFINED

#include <cassert>
#include <type_traits>
#include <utility>

namespace Trinity {
/// A helper class for creating a zero allocation list which elements are
/// placed on the stack directly. Elements are of the same type, and the
/// references are managed automatically upon creation or deletion.
///
/// The list always grows from left to right.
///
/// \attention This class is threadunsafe and may only be used
///            from the same thread!
template <typename Child>
class StackListReference
{
    Child* left_ = nullptr;
    Child* right_ = nullptr;

  public:
    /// Creates a new and empty stack list which origin is this element
    constexpr StackListReference() = default;
    /// Adds a new element to the
    explicit constexpr StackListReference(Child* left) noexcept
        : left_(GetLastEntryOf(left))
    {
        assert(left);
        assert(left->right_ == nullptr);
        left->right_ = this;
    }
    ~StackListReference() noexcept { Unlink(); }

    explicit constexpr StackListReference(StackListReference const&) = delete;
    explicit constexpr StackListReference(StackListReference&& right) noexcept
        : left_(std::exchange(right.left_, nullptr)),
          right_(std::exchange(right.right_, nullptr))
    {
        /*if (ref_)
        {
            ref_->ref_ = static_cast<Child*>(this);
        }*/
    }

    constexpr StackListReference& operator=(StackListReference const&) = delete;
    constexpr StackListReference& operator=(StackListReference&& right) noexcept
    {
        /*Unlink();
        if ((ref_ = std::exchange(right.ref_, nullptr)))
        {
            ref_->ref_ = static_cast<Child*>(this);
        }*/
        return *this;
    }

  protected:
    constexpr void Unlink() noexcept
    {
        /*assert(!right_ || left_);
        if (left_)
        {
            ref_->ref_ = nullptr;
        }*/
    }

    /*constexpr bool HasRef() const noexcept { return bool(ref_); }

    constexpr Child* GetRef() noexcept
    {
        .assert(HasRef());
         return ref_;
    }
    constexpr Referenced const* GetRef() const noexcept
    {
        assert(HasRef());
        return ref_;
    }*/

  private:
    static constexpr Child* GetLastEntryOf(Child* entry) noexcept
    {
        assert(entry);
        assert(entry->right_);

        while (entry->right_ != nullptr)
        {
            entry = entry->right_;
        }
        return entry;
    }
};
} // namespace Trinity

#endif // TRINITY_ASYNC_STACK_REFERENCE_HPP_DEFINED
