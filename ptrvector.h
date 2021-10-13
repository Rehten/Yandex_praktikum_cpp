#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

template <typename T>
class PtrVector {
  public:
    PtrVector() = default;

    // Создаёт вектор указателей на копии объектов из other
    PtrVector(const PtrVector& other) {
      T *ptr_buf = nullptr;

      items_.reserve(other.items_.size());

      for (const T *ptr : other.items_)
      {
        ptr_buf = ptr ? new T(*ptr) : nullptr;
        
        items_.push_back(ptr_buf);
        ptr_buf = nullptr;
      }
    }

    // Деструктор удаляет объекты в куче, на которые ссылаются указатели,
    // в векторе items_
    ~PtrVector() {
      Free();
    }

    // Возвращает ссылку на вектор указателей
    vector<T*>& GetItems() noexcept {
      return items_;
    }

    // Возвращает константную ссылку на вектор указателей
    vector<T*> const& GetItems() const noexcept {
      return items_;
    }

  private:
    vector<T*> items_{};

    void Free()
    {
      for (T *ptr : items_)
      {
        delete ptr;
      }
    }
};