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

#ifndef TRINITY_WEAK_INTRUSIVE_PTR_HPP_DEFINED
#define TRINITY_WEAK_INTRUSIVE_PTR_HPP_DEFINED

#include <cassert>
#include <type_traits>
#include <utility>

#include <boost/intrusive_ptr.hpp>

namespace Trinity {
/// A smart pointer similar to boost::intrusive_ptr, however it
/// additionally handles multiple types of reference counters such as
/// a possible weak and strong counter.
///
/// This class is qualifier correct, so the pointer of the wrapped object
/// is qualified depending on the qualification of this class.
///
/// For performance reason the counter isn't implicitily copyable and
/// must be explicitly copied through a call to IntrusivePtr::Copy().
template <typename T, typename Type, Type Value>
class IntrusivePtr
{
    T* ptr_ = nullptr;

  public:
    constexpr IntrusivePtr() noexcept = default;
    explicit constexpr IntrusivePtr(std::nullptr_t) noexcept : ptr_(nullptr) {}
    constexpr IntrusivePtr(T* ptr, bool add_ref = true) noexcept(
        noexcept(IncreaseRefCounter(std::declval<T*>(), Value)))
        : ptr_(ptr)
    {
        assert(ptr && ptr_);
        if (ptr_ && add_ref)
        {
            IncreaseRefCounter(ptr_, Value);
        }
    }
    constexpr IntrusivePtr(IntrusivePtr const&) = delete;
    constexpr IntrusivePtr(IntrusivePtr&& right) noexcept
        : ptr_(std::exchange(right.ptr_, nullptr))
    {
    }
    ~IntrusivePtr()
    {
        if (ptr_)
        {
            DecreaseRefCounter(ptr_, Value);
        }
    }
    constexpr IntrusivePtr& operator=(IntrusivePtr const&) = delete;
    constexpr IntrusivePtr& operator=(IntrusivePtr&& right) noexcept(
        noexcept(DecreaseRefCounter(std::declval<T*>(), Value)))
    {
        if (auto old = std::exchange(ptr_, right.ptr_))
        {
            DecreaseRefCounter(old, Value);
        }
        right.ptr_ = nullptr;
        return *this;
    }
    IntrusivePtr& operator=(std::nullptr_t) noexcept
    {
        if (auto old = std::exchange(ptr_, nullptr))
        {
            DecreaseRefCounter(old, Value);
        }
        return *this;
    }

    constexpr IntrusivePtr& operator=(T* ptr)
    {
        if (auto old = std::exchange(ptr_, ptr))
        {
            DecreaseRefCounter(old, Value);
        }
        if (ptr_)
        {
            IncreaseRefCounter(ptr_, Value);
        }
        return *this;
    }

    /// Copies the IntrusivePtr explicitly
    IntrusivePtr Copy() { return IntrusivePtr(ptr_); }

    constexpr T* operator->() noexcept { return ptr_; }
    constexpr T const* operator->() const noexcept { return ptr_; }

    constexpr T* Get() noexcept { return ptr_; }
    constexpr T const* Get() const noexcept { return ptr_; }

    explicit operator bool() const { return bool(ptr_); }
};

template <typename T, typename Slot>
void IncreaseRefCounter(T* /*ptr*/, Slot&& /*slot*/) noexcept
{
    static_assert(!std::is_same<T, T>::value,
                  "IncreaseRefCounter is not implemented for this class!");
}
template <typename T, typename Slot>
void DecreaseRefCounter(T* /*ptr*/, Slot&& /*slot*/) noexcept
{
    static_assert(!std::is_same<T, T>::value,
                  "DecreaseRefCounter is not implemented for this class!");
}

/// Defines a generic intrusive pointer type which has a strong and
/// a weak reference counter.
enum class StrongWeakType
{
    Strong,
    Weak
};
} // namespace Trinity

#endif // TRINITY_WEAK_INTRUSIVE_PTR_HPP_DEFINED
