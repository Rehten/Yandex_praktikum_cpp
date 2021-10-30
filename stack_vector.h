#pragma once

#include <array>
#include <string>
#include <stdexcept>

template<typename T, size_t N>
class StackVector
{
  private:
    using invalid_argument = std::invalid_argument;
    using overflow_error = std::overflow_error;
    using underflow_error = std::underflow_error;

    T items_[N];
    size_t size_ = 0;

  public:
    class Iterator
    {
      private:
        T *item_{};
      public:
        using ConstIterator = typename StackVector<const T, N>::Iterator;

        Iterator(T *item): item_(item)
        {};

        Iterator(T &item): item_(&item)
        {};

        bool operator ==(const Iterator &iterator)
        {
          return item_ == iterator.item_;
        }

        bool operator ==(T *ptr)
        {
          return item_ == ptr;
        }

        bool operator !=(const Iterator &iterator)
        {
          return !(item_ == iterator.item_);
        }

        bool operator !=(T *ptr)
        {
          return item_ != ptr;
        }

        T &operator *()
        {
          return *item_;
        }

        Iterator &operator ++(int)
        {
          ++item_;

          return *this;
        }

        Iterator operator ++()
        {
          Iterator copy = item_;

          ++item_;

          return copy;
        }

        Iterator &operator --(int)
        {
          --item_;

          return *this;
        }

        Iterator operator --()
        {
          Iterator copy = item_;

          --item_;

          return copy;
        }

        Iterator &operator +(int num)
        {
          item_ += num;

          return *this;
        }

        Iterator &operator -(int num)
        {
          item_ -= num;

          return *this;
        }
    };

    explicit StackVector(size_t a_size = 0): size_(a_size)
    {
      if (size_ > N)
      {
        throw invalid_argument("Size is more than can allocated.");
      }
    };

    T &operator[](size_t index)
    {
      return items_[index];
    };

    const T &operator[](size_t index) const
    {
      return items_[index];
    };

    Iterator begin()
    {
      return items_[0];
    };

    Iterator end()
    {
      return items_[N - 1];
    };

    typename StackVector<const T, N>::Iterator begin() const
    {
      return items_[0];
    };
    typename StackVector<const T, N>::Iterator end() const
    {
      return items_[N - 1];
    };
    size_t Size() const
    {
      return size_;
    };

    size_t Capacity() const
    {
      return N;
    };

    void PushBack(const T &value)
    {
      if (size_ == N)
      {
        throw overflow_error("Cannot allocate more memory");
      }

      items_[size_++] = value;
    };

    T PopBack()
    {
      if (!size_)
      {
        throw underflow_error("Cannot pop empty array");
      }

      return items_[--size_];
    };
};
