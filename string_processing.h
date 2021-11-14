#pragma once

#include <vector>
#include <set>
#include <string>

std::vector<std::string> SplitIntoWords(const std::string &text);

template<typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer &strings)
{
  std::set<std::string> non_empty_strings;

  for (const auto &str : strings)
  {
    if (str.begin() != str.end())
    {
      non_empty_strings.insert({str.begin(), str.end()});
    }
  }
  return non_empty_strings;
}
