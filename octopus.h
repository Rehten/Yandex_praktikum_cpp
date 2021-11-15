#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "ptrvector.h"

using namespace std;

// Щупальце
class Tentacle {
  public:
    explicit Tentacle(int id) noexcept
      : id_(id) {
    }

    int GetId() const noexcept {
      return id_;
    }

    Tentacle* GetLinkedTentacle() const noexcept {
      return linked_tentacle_;
    }
    void LinkTo(Tentacle& tentacle) noexcept {
      linked_tentacle_ = &tentacle;
    }
    void Unlink() noexcept {
      linked_tentacle_ = nullptr;
    }

  private:
    int id_ = 0;
    Tentacle* linked_tentacle_ = nullptr;
};

// Осьминог
class Octopus {
  public:
    Octopus()
      : Octopus(8) {
    }

    explicit Octopus(int num_tentacles) {
      Tentacle* t = nullptr;

      try {
        tentacles_.GetItems().reserve(num_tentacles);

        for (int i = 1; i <= num_tentacles; ++i) {
          t = new Tentacle(i);      // Может выбросить исключение bad_alloc
          tentacles_.GetItems().push_back(t);  // Может выбросить исключение bad_alloc

          // Обнуляем указатель на щупальце, которое уже добавили в tentacles_,
          // чтобы не удалить его в обработчике catch повторно
          t = nullptr;
        }
      } catch (const bad_alloc&) {
        // Удаляем щупальце, которое создали, но не добавили в tentacles_
        delete t;
        // Конструктор не смог создать осьминога с восемью щупальцами,
        // поэтому выбрасываем исключение, чтобы сообщить вызывающему коду об ошибке
        // throw без параметров внутри catch выполняет ПЕРЕВЫБРОС пойманного исключения
        throw;
      }
    }

    int GetTentacleCount() const noexcept {
      return static_cast<int>(tentacles_.GetItems().size());
    }

    const Tentacle& GetTentacle(size_t index) const {
      return *tentacles_.GetItems()[index];
    }
    Tentacle& GetTentacle(size_t index) {
      return *tentacles_.GetItems()[index];
    }

    Tentacle &AddTentacle()
    {
      int last_id = tentacles_.GetItems().size() ? tentacles_.GetItems()[tentacles_.GetItems().size() - 1]->GetId() : 1;
      auto last_tentacle = new Tentacle(last_id + 1);

      tentacles_.GetItems().push_back(last_tentacle);

      return *last_tentacle;
    }

  private:
    PtrVector<Tentacle> tentacles_;
};