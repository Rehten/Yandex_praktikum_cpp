#pragma once

#include "string_processing.h"

using std::set;
using std::vector;
using std::string;

vector<string> SplitIntoWords(const string &text)
{
  vector<string> words;
  string word;
  for (const char c : text)
  {
    if (c == ' ')
    {
      if (!word.empty())
      {
        words.push_back(word);
        word.clear();
      }
    }
    else
    {
      word += c;
    }
  }
  if (!word.empty())
  {
    words.push_back(word);
  }

  return words;
}

template<typename StringContainer>
set<string> MakeUniqueNonEmptyStrings(const StringContainer &strings)
{
  set<string> non_empty_strings;
  for (const string &str : strings)
  {
    if (!str.empty())
    {
      non_empty_strings.insert(str);
    }
  }
  return non_empty_strings;
}

template set<string> MakeUniqueNonEmptyStrings<vector<string>>(const vector<string> &strings);
