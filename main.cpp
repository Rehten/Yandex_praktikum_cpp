#include <algorithm>
#include <execution>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <future>

using namespace std;

template <typename Strategy, typename ForwardRange, typename Function>
void ForEach(Strategy, ForwardRange& range, Function function) {
  size_t range_size = range.size();
  size_t tasks_count {};

  if (range_size < 20 || is_same_v<Strategy, execution::sequenced_policy>)
  {
    tasks_count = 1;

    for_each(execution::seq, range.begin(), range.end(), function);

    return;
  }
  else if (range_size < 100)
  {
    tasks_count = 2;
  }
  else if (range_size < 1000)
  {
    tasks_count = 3;
  }
  else if (range_size < 10000)
  {
    tasks_count = 4;
  }
  else
  {
    tasks_count = 5;
  }

  vector<future<void>> futures {};
  vector<typename ForwardRange::iterator> iters {};
  size_t iters_added{};
  size_t i{};

  futures.reserve(tasks_count);
  iters.reserve(tasks_count + 1);


  for (auto iter = range.begin(); iter != range.end(); ++iter)
  {
    if ((i / (iters_added ? iters_added : 1)) * tasks_count >= range_size)
    {
      iters.push_back(iter);
      ++iters_added;
    }

    ++i;
  }

  iters.push_back(range.end());

  for (size_t i = 0; i != tasks_count; ++i)
  {
    futures.push_back(async([&iters, i, &function] {
      for_each(iters[i], iters[i + 1], function);
    }));
  }

  for_each(futures.begin(),  futures.end(), [](future<void> &fut) -> void {
    fut.wait();
  });

}

template <typename ForwardRange, typename Function>
void ForEach(ForwardRange& range, Function function) {
  ForEach(execution::seq, range, function);
}

int main() {
  // для итераторов с произвольным доступом тоже должно работать
  vector<string> strings = {"cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code","cat", "dog", "code",};

  ForEach(strings, [](string& s) { reverse(s.begin(), s.end()); });

  for (string_view s : strings) {
    cout << s << " ";
  }
  cout << endl;

  // вывод: tac god edoc

  return 0;
}