////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2024 Evan Bowman
//
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/. */
//
////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "number/int.h"
#include <array>
#include <new>



template <typename T, u32 Capacity> struct TinyBuffer
{
    alignas(T) std::array<u8, Capacity * sizeof(T)> mem_;
    u8 count_ = 0;
    using ValueType = T;


    T* data()
    {
        return (T*)mem_.data();
    }


    TinyBuffer() = default;


    static constexpr u8 capacity()
    {
        return Capacity;
    }


    TinyBuffer(const TinyBuffer&) = default;
    TinyBuffer& operator=(const TinyBuffer&) = default;


    ~TinyBuffer()
        requires(!std::is_trivially_destructible_v<T>)
    {
        clear();
    }


    ~TinyBuffer()
        requires(std::is_trivially_destructible_v<T>)
    = default;


    bool full() const
    {
        return count_ == Capacity;
    }


    T& operator[](u32 index)
    {
        return ((T*)mem_.data())[index];
    }


    const T& operator[](u32 index) const
    {
        return ((T*)mem_.data())[index];
    }


    bool push_back(const T& elem)
    {
        if (full()) {
            return false;
        }
        new ((T*)mem_.data() + count_) T(elem);
        ++count_;
        return true;
    }


    T& back() const
    {
        return *((T*)mem_.data() + (count_ - 1));
    }


    void pop_back()
    {
        if (empty()) {
            return;
        }
        if constexpr (!std::is_trivially_destructible_v<T>) {
            ((T*)mem_.data() + --count_)->~T();
        } else {
            --count_;
        }
    }


    u8 size() const
    {
        return count_;
    }


    bool empty() const
    {
        return size() == 0;
    }


    void clear()
    {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            while (count_) {
                pop_back();
            }
        } else {
            count_ = 0;
        }
    }
};
