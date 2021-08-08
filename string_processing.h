#pragma once

#include <vector>
#include <set>
#include <string>

std::vector<std::string> SplitIntoWords(const std::string &text);

template<typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer &strings);
