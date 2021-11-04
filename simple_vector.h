#pragma once

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <iterator>
#include <initializer_list>

#include "array_ptr.h"

template<typename Type>
class SimpleVector
{
  private:
    using out_of_range = std::out_of_range;
    ArrayPtr<Type> ptr_;
    size_t size_ = 0;
    size_t capacity_ = 0;
  public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;

    SimpleVector(std::pair<size_t, size_t> reserve_capacity): size_(reserve_capacity.first), capacity_(reserve_capacity.second)
    {}

    explicit SimpleVector(size_t capacity) : SimpleVector(capacity, 0)
    {}

    SimpleVector(size_t size, Type &&value) : size_(size), capacity_(size_)
    {
      ArrayPtr<Type> ptr(size_);

      ptr_.swap(ptr);

      for (size_t i = 0; i != size_; ++i)
      {
        ptr_.Get()[i] = std::move(value);
      }
    }

    SimpleVector(std::initializer_list<Type> init) : SimpleVector(init.size())
    {
      size_t index{};

      for (const Type &val: init)
      {
        ptr_.Get()[index] = val;
        ++index;
      }
    }

    SimpleVector(const SimpleVector &other) : SimpleVector(other.size_)
    {
      std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector &&other)  noexcept
    {
      swap(other);
    }

    SimpleVector &operator=(SimpleVector &&rhs)
    {
      if (this == &rhs)
      {
        return *this;
      }

      SimpleVector<Type> copy(std::move(rhs));

      swap(copy);

      return *this;
    }

    void PushBack(const Type &item)
    {
      Insert(end(), item);
    }

    void PushBack(Type &&item)
    {
      Insert(end(), std::move(item));
    }

    Iterator Insert(ConstIterator pos, const Type &value)
    {
      size_t inserted_index = pos - begin();

      if (size_ + 1 <= capacity_)
      {
        ptr_.Get()[size_] = value;

        for (size_t i = size_; i != size_ - (end() - pos); --i)
        {
          std::swap(ptr_.Get()[i], ptr_.Get()[i - 1]);
        }
        ++size_;
      }
      else
      {
        SimpleVector<Type> copy(capacity_ ? capacity_ * 2 : 1);

        copy.size_ = size_ + 1;

        if (GetSize())
        {
          size_t moved{};

          for (size_t i = 0; i != copy.size_; ++i)
          {
            if (i == inserted_index)
            {
              copy.ptr_.Get()[i] = value;
              ++moved;
            }

            copy.ptr_.Get()[i + moved] = *(begin() + i);
          }
        }
        else
        {
          copy.ptr_.Get()[0] = value;
        }

        swap(copy);
      }

      return begin() + inserted_index;
    }

    Iterator Insert(ConstIterator pos, Type &&value)
    {
      size_t inserted_index = pos - begin();

      if (size_ + 1 > capacity_)
      {
        Resize(size_ + 1);
      }
      else
      {
        ++size_;
      }

      // @TODO: Пока что основная проблема здесь
      for (size_t i = (size_ - 1); i > inserted_index; --i)
      {
        ptr_.Get()[i] = std::exchange(ptr_.Get()[i - 1], {});
      }

      ptr_.Get()[inserted_index] = std::move(value);

      return begin() + inserted_index;
    }

    void Reserve(size_t new_capacity)
    {
      if (new_capacity > capacity_)
      {
        SimpleVector<Type> copy(new_capacity);
        copy.size_ = size_;

        std::copy(begin(), end(), copy.begin());

        swap(copy);
      }
    }

    void PopBack() noexcept
    {
      if (size_)
      {
        --size_;
      }
    }

    Iterator Erase(ConstIterator pos)
    {
      // Напишите тело самостоятельно

      --size_;
      for (size_t erased_index = pos - begin(); erased_index != size_; ++erased_index)
      {
        std::swap(ptr_.Get()[erased_index], ptr_.Get()[erased_index + 1]);
      }

      return const_cast<Iterator>(pos);
    }

    void swap(SimpleVector &other) noexcept
    {
      ptr_.swap(other.ptr_);
      std::swap(capacity_, other.capacity_);
      std::swap(size_, other.size_);
    }

    size_t GetSize() const noexcept
    {
      return size_;
    }

    size_t GetCapacity() const noexcept
    {
      return capacity_;
    }

    bool IsEmpty() const noexcept
    {
      return !GetSize();
    }

    Type &operator[](size_t index) noexcept
    {
      return ptr_.Get()[index];
    }

    const Type &operator[](size_t index) const noexcept
    {
      return ptr_.Get()[index];
    }

    Type &At(size_t index)
    {
      if (index >= size_)
      {
        throw out_of_range("Out of range of vector");
      }

      return (*this)[index];
    }

    const Type &At(size_t index) const
    {
      if (index >= size_)
      {
        throw out_of_range("Out of range of vector");
      }

      return (*this)[index];
    }

    void Clear() noexcept
    {
      Resize(0);
    }

    void Resize(size_t new_size)
    {
      if (size_ < new_size)
      {
        SimpleVector<Type> copy(new_size);

        for (size_t i = 0; i != size_; ++i)
        {
          copy.ptr_.Get()[i] = std::move(ptr_.Get()[i]);
        }

        delete ptr_.Release();

        swap(copy);
      }
      else if (size_ > new_size)
      {
        size_ = new_size;
      }
    }

    Iterator begin() noexcept
    {
      return ptr_.Get();
    }

    Iterator end() noexcept
    {
      return ptr_.Get() + size_;
    }

    ConstIterator begin() const noexcept
    {
      // Напишите тело самостоятельно
      return cbegin();
    }

    ConstIterator end() const noexcept
    {
      return cend();
    }

    ConstIterator cbegin() const noexcept
    {
      return ptr_.Get();
    }

    ConstIterator cend() const noexcept
    {
      return ptr_.Get() + size_;
    }
};


template<typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
  return !(lhs == rhs);
}

template<typename Type>
inline bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());;
}

template<typename Type>
inline bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
  return (lhs < rhs) || (lhs == lhs);
}

template<typename Type>
inline bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
  return (rhs < lhs);
}

template<typename Type>
inline bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
  return (rhs < lhs) || (rhs == lhs);
}

std::pair<size_t, size_t> Reserve(size_t n)
{
  return {0, n};
}
