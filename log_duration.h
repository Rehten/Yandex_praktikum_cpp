#pragma once

#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define LOG_DURATION_STREAM(x, stream) LogDuration UNIQUE_VAR_NAME_PROFILE(x, stream)

class LogDuration {
  public:
    using Clock = std::chrono::steady_clock;

    LogDuration(std::basic_string_view<char> id, std::ostream &ostream = std::cerr) : id_(id), start_time_(Clock::now()), os_(ostream)
    {

    }

    ~LogDuration()
    {
      using namespace std::chrono;
      using namespace std::literals;

      const auto end_time = Clock::now();
      const auto dur = end_time - start_time_;
      os_ << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

  private:
    const std::string id_;
    const Clock::time_point start_time_;

    std::ostream &os_;
};
