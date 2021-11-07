#include "string_processing.h"

using std::set;
using std::vector;
using std::string;
using std::string_view;

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
