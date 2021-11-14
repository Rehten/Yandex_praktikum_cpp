#pragma once
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <algorithm>

#include "log_duration.h"
#include "test_runner_p.h"

template <typename Key, typename Value>
class ConcurrentMap {
  public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct DictionaryWithMutes
    {
        std::map<Key, Value> dictionary;
        std::mutex m;
    };

    struct Access {
      private:
        std::lock_guard<std::mutex &> guard_;
      public:
        Value &ref_to_value;

        Access(DictionaryWithMutes *src, Key key): guard_(src->m), ref_to_value(src->dictionary[static_cast<uint64_t>(key)])
        {}
    };

    explicit ConcurrentMap(size_t bucket_count): data_(bucket_count)
    {}

    Access operator[](const Key& key)
    {
      uint64_t index = static_cast<uint64_t>(key) % data_.size();

      return {&data_[index], key};
    }

    std::map<Key, Value> BuildOrdinaryMap()
    {
      std::map<Key, Value> rslt{};

      for (uint64_t i = 0; i != static_cast<uint64_t>(data_.size()); ++i)
      {
        std::lock_guard<std::mutex> guard(data_[i].m);
        for (auto iter = data_[i].dictionary.begin(); iter != data_[i].dictionary.end(); ++iter)
        {
          rslt[iter->first] = iter->second;
        }
      }

      return rslt;
    }

  private:
    std::vector<DictionaryWithMutes> data_;
};
