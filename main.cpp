#include <iostream>
#include <numeric>
#include <utility>
#include <vector>
#include <mutex>
#include <algorithm>
#include <execution>

using namespace std;

template <typename Container, typename Predicate>
vector<typename Container::value_type> CopyIfUnordered(const Container& container, Predicate predicate) {
  vector<typename Container::value_type> result;
  result.reserve(container.size());
  mutex m{};

  for_each(execution::par, container.begin(), container.end(), [&predicate, &result, &m](const typename Container::value_type &value) {
    if (predicate(value)) {
      m.lock();
      result.push_back(value);
      m.unlock();
    }
  });

  return result;
}

int main() {
  vector<int> numbers(1'000);
  iota(numbers.begin(), numbers.end(), 0);

  const vector<int> even_numbers =
    CopyIfUnordered(numbers, [](int number) { return number % 2 == 0; });
  for (const int number : even_numbers) {
    cout << number << " "s;
  }
  cout << endl;
  // выведет все чётные числа от 0 до 999
}