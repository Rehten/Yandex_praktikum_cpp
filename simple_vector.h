#pragma once

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <initializer_list>

#include "array_ptr.h"

template<typename Type>
class SimpleVector
{
    using out_of_range = std::out_of_range;

  private:
    ArrayPtr<Type> ptr_;
    size_t size_ = 0;
    size_t capacity_ = 0;
  public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : SimpleVector(size, 0)
    {}

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type &value) : size_(size), capacity_(size_)
    {
      ArrayPtr<Type> ptr(size_);

      ptr_.swap(ptr);
      // Напишите тело конструктора самостоятельно
      for (size_t i = 0; i != size_; ++i)
      {
        ptr_.Get()[i] = value;
      }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init): SimpleVector(init.size())
    {
      size_t index {};

      for (const Type &val : init)
      {
        ptr_.Get()[index] = val;
        ++index;
      }
    }

    SimpleVector(const SimpleVector& other): SimpleVector(other.size_) {
      std::copy(other.begin(),  other.end(), begin());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
      // Напишите тело конструктора самостоятельно
      if (this == &rhs)
      {
        return *this;
      }

      SimpleVector<Type> copy (rhs);

      swap(copy);

      return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
      Insert(end(), item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
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
        SimpleVector<Type> copy (capacity_ ? capacity_ * 2 : 1);

        copy.size_ = size_ + 1;

        if (GetSize())
        {
          size_t moved {};

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

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
      // Напишите тело самостоятельно
      if (size_)
      {
        --size_;
      }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
      // Напишите тело самостоятельно

      for (size_t erased_index = pos - begin(); erased_index != size_ - 1; ++erased_index)
      {
        std::swap(ptr_.Get()[erased_index], ptr_.Get()[erased_index + 1]);
      }

      return const_cast<Iterator>(pos);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
      ptr_.swap(other.ptr_);
      std::swap(capacity_, other.capacity_);
      std::swap(size_, other.size_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept
    {
      // Напишите тело самостоятельно
      return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept
    {
      // Напишите тело самостоятельно
      return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept
    {
      return !GetSize();
    }

    // Возвращает ссылку на элемент с индексом index
    Type &operator[](size_t index) noexcept
    {
      // Напишите тело самостоятельно
      return ptr_.Get()[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type &operator[](size_t index) const noexcept
    {
      // Напишите тело самостоятельно
      return ptr_.Get()[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type &At(size_t index)
    {
      // Напишите тело самостоятельно
      if (index >= size_)
      {
        throw out_of_range("Out of range of vector");
      }

      return (*this)[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type &At(size_t index) const
    {
      // Напишите тело самостоятельно
      if (index >= size_)
      {
        throw out_of_range("Out of range of vector");
      }

      return (*this)[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept
    {
      Resize(0);
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size)
    {
      // Напишите тело самостоятельно
      if (size_ < new_size)
      {
        SimpleVector<Type> copy (new_size);

        for (size_t i = 0; i != size_; ++i)
        {
          copy.ptr_.Get()[i] = ptr_.Get()[i];
        }

        delete ptr_.Release();

        swap(copy);
      }
      else if (size_ > new_size)
      {
        size_ = new_size;
      }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept
    {
      // Напишите тело самостоятельно
      return ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept
    {
      // Напишите тело самостоятельно
      return ptr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept
    {
      // Напишите тело самостоятельно
      return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept
    {
      // Напишите тело самостоятельно
      return cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept
    {
      // Напишите тело самостоятельно
      return ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept
    {
      // Напишите тело самостоятельно
      return ptr_.Get() + size_;
    }
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
  return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());;
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
  return (lhs < rhs) || (lhs == lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
  return (rhs < lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
  return (rhs < lhs) || (rhs == lhs);
}
