#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <future>
#include <utility>
#include <numeric>

using namespace std;

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

struct Stats {
    map<string, int> word_frequences;

    void operator+=(const Stats& other) {
      for (auto &pr : other.word_frequences)
      {
        if (word_frequences.count(pr.first))
        {
          word_frequences[pr.first] += pr.second;
        }
        else
        {
          word_frequences[pr.first] = pr.second;
        }
      }
    }
};

using KeyWords = set<string, less<>>;

Stats ExploreKeyWords(const KeyWords& key_words, istream& input) {
  string input_content;
  vector<vector<string>> lines{};

  input_content.reserve(100);

  while (input)
  {
    input_content += static_cast<char>(input.get());

    if (input_content[input_content.size() - 1] == '\n')
    {
      lines.push_back(SplitIntoWords(input_content));
      input_content = ""s;
    }
  }

  Stats rslt{};
  vector<Stats> stats_collection{lines.size()};

  for (auto &word : key_words)
  {
    rslt.word_frequences[word];
  }

  for (size_t i = 0; i != stats_collection.size(); ++i)
  {
    Stats async{};

    for (auto &word : lines[i])
    {
      if (rslt.word_frequences.count(word))
      {
        ++async.word_frequences[word];
      }
    }

    stats_collection[i] = async;
  }

  for (size_t i = 0; i != stats_collection.size(); ++i)
  {
    rslt += stats_collection[i];
  }

  return rslt;
}

int main() {
  const KeyWords key_words = {"yangle", "rocks", "sucks", "all"};

  stringstream ss;
  ss << "this new yangle service really rocks\n";
  ss << "It sucks when yangle isn't available\n";
  ss << "10 reasons why yangle is the best IT company\n";
  ss << "yangle rocks others suck\n";
  ss << "Goondex really sucks, but yangle rocks. Use yangle\n";

  for (const auto& [word, frequency] : ExploreKeyWords(key_words, ss).word_frequences) {
    cout << word << " " << frequency << endl;
  }

  return 0;
}
