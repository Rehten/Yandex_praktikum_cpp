#include "log_duration.h"

using Clock = std::chrono::steady_clock;

LogDuration::LogDuration(const std::string &id, std::ostream &ostream = std::cerr)
  : id_(id), start_time_(::Clock::now()), os_(ostream)
{
}

LogDuration::~LogDuration()
{
  using namespace std::chrono;
  using namespace std::literals;

  const auto end_time = Clock::now();
  const auto dur = end_time - start_time_;
  os_ << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
}
