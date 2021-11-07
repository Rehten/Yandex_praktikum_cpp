#include "search_server.h"
#include <algorithm>
#include <string_view>

#include <cmath>

using std::string;
using std::string_view;
using std::tuple;
using std::set;
using std::map;
using std::vector;
using std::invalid_argument;
using std::all_of;
using std::find;

SearchServer::SearchServer(const string_view &stop_words_text) : SearchServer(SplitIntoWords(string(stop_words_text.begin(),  stop_words_text.end())))
{}

SearchServer::SearchServer(const string &stop_words_text) : SearchServer(SplitIntoWords(stop_words_text))
{}

void
SearchServer::AddDocument(int document_id, const string_view &document, DocumentStatus status, const vector<int> &ratings)
{
  if ((document_id < 0) || (documents_.count(document_id) > 0))
  {
    throw invalid_argument("Invalid document_id");
  }
  const auto words = SplitIntoWordsNoStop({document.begin(),  document.end()});

  const double inv_word_count = 1.0 / words.size();
  for (const string &word: words)
  {
    word_to_document_freqs_[word][document_id] += inv_word_count;
  }
  documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
  document_ids_.push_back(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const string_view &raw_query, DocumentStatus status) const
{
  return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
    return document_status == status;
  });
}

vector<Document> SearchServer::FindTopDocuments(const string_view &raw_query) const
{
  return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const
{
  return documents_.size();
}

vector<int>::iterator SearchServer::begin()
{
  return document_ids_.begin();
}

vector<int>::iterator SearchServer::end()
{
  return document_ids_.end();
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const string_view &raw_query, int document_id) const
{
  // Empty result by initializing it with default constructed tuple
  using namespace std::literals;

  Query query;
  if (!ParseQuery(string{raw_query.begin(),  raw_query.end()}, query))
  {
    throw invalid_argument("Invalid raw query!"s);
  }

  for (const string &word: query.minus_words)
  {
    if (word_to_document_freqs_.count(word) == 0)
    {
      continue;
    }
    if (word_to_document_freqs_.at(word).count(document_id))
    {
      return {{}, documents_.at(document_id).status};
    }
  }

  vector<string_view> matched_words;
  for (const string &word: query.plus_words)
  {
    if (word_to_document_freqs_.count(word) == 0)
    {
      continue;
    }
    if (word_to_document_freqs_.at(word).count(document_id))
    {
      matched_words.push_back(word);
    }
  }

  return {matched_words, documents_.at(document_id).status};
}

const int SearchServer::MaxResultDocumentCount = 5;

bool SearchServer::IsStopWord(const string &word) const
{
  return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const string &word)
{
  return none_of(word.begin(), word.end(), [](char c) {
    return c >= '\0' && c < ' ';
  });
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string &text) const
{
  vector<string> words;
  for (const string &word: SplitIntoWords(text))
  {
    if (!IsValidWord(word))
    {
      throw invalid_argument(string("Word ") + word + string(" is invalid"));
    }
    if (!IsStopWord(word))
    {
      words.push_back(word);
    }
  }
  return words;
}

int SearchServer::ComputeAverageRating(const vector<int> &ratings)
{
  if (ratings.empty())
  {
    return 0;
  }
  int rating_sum = 0;
  for (const int rating: ratings)
  {
    rating_sum += rating;
  }
  return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const string &text) const
{
  if (text.empty())
  {
    throw invalid_argument("Query word is empty");
  }
  string word = text;
  bool is_minus = false;
  if (word[0] == '-')
  {
    is_minus = true;
    word = word.substr(1);
  }
  if (word.empty() || word[0] == '-' || !IsValidWord(word))
  {
    throw invalid_argument(string("Query word ") + text + " is invalid");
  }

  return {word, is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(const string &text) const
{
  Query result;
  for (const string &word: SplitIntoWords(text))
  {
    const auto query_word = ParseQueryWord(word);
    if (!query_word.is_stop)
    {
      if (query_word.is_minus)
      {
        result.minus_words.insert(query_word.data);
      }
      else
      {
        result.plus_words.insert(query_word.data);
      }
    }
  }
  return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string &word) const
{
  return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

const map<string_view, double> &SearchServer::GetWordFrequencies(int document_id) const
{
  static map<std::string_view, double> rslt;

  rslt = {};

  for (const auto &[word, freqs]: word_to_document_freqs_)
  {
    if (freqs.count(document_id))
    {
      rslt.insert({word, freqs.at(document_id)});
    }
  }

  return rslt;
}

void SearchServer::RemoveDocument(int document_id)
{
  document_ids_.erase(find(document_ids_.begin(), document_ids_.end(), document_id));
  documents_.erase(document_id);

  for (auto &[word, freqs]: word_to_document_freqs_)
  {
    if (freqs.count(document_id))
    {
      freqs.erase(document_id);
    }
  }
}

bool SearchServer::ParseQueryWord(string text, SearchServer::QueryWord &result) const
{
  // Empty result by initializing it with default constructed QueryWord
  result = {};

  if (text.empty())
  {
    return false;
  }
  bool is_minus = false;
  if (text[0] == '-')
  {
    is_minus = true;
    text = text.substr(1);
  }
  if (text.empty() || text[0] == '-' || !IsValidWord(text))
  {
    return false;
  }

  result = QueryWord{text, is_minus, IsStopWord(text)};
  return true;
}

bool SearchServer::ParseQuery(const string &text, SearchServer::Query &result) const
{
  // Empty result by initializing it with default constructed Query
  result = {};
  for (const string &word: SplitIntoWords(text))
  {
    QueryWord query_word;
    if (!ParseQueryWord(word, query_word))
    {
      return false;
    }
    if (!query_word.is_stop)
    {
      if (query_word.is_minus)
      {
        result.minus_words.insert(query_word.data);
      }
      else
      {
        result.plus_words.insert(query_word.data);
      }
    }
  }
  return true;
}
