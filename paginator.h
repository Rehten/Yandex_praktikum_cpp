#pragma once

#include <vector>
#include "document.h"

class Paginator : public std::vector<std::vector<Document>>
{
  private:
    size_t page_size_;
  public:
    template<typename I>
    Paginator(I start, I end, size_t page_size);
};

template<typename Container>
auto Paginate(const Container &c, size_t page_size);


/**
 * По идее этот код реализации должен быть в paginator.cpp, ноэтого файла нет в тренажере
 */
template<typename I>
Paginator::Paginator(I start, I end, size_t page_size) : page_size_(page_size)
{
  int i{};
  std::vector<Document> v{};

  for (I iter = start; iter != end; ++iter)
  {
    ++i;
    v.push_back(*iter);

    if (i == page_size_)
    {
      push_back({v.begin(), v.end()});
      v.clear();
      i = 0;
    }
  }
}

template<typename Container>
auto Paginate(const Container &c, size_t page_size)
{
  return Paginator(c.begin(), c.end(), page_size);
}
