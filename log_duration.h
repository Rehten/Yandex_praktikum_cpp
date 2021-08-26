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

  LogDuration(const std::string& id, std::ostream &ostream);

  ~LogDuration();

private:
  const std::string id_;
  const Clock::time_point start_time_;

  std::ostream &os_;
};
