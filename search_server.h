#pragma once

#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>

#include "read_input_functions.h"
#include "string_processing.h"
#include "paginator.h"
#include "log_duration.h"

class SearchServer
{
public:
  template<typename StringContainer>
  SearchServer(const StringContainer &stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
  {
    using namespace std;
    using namespace literals;
    for (auto &word: stop_words_)
    {
      if (!IsValidWord(word))
      {
        throw invalid_argument("Word "s + word + "is invalid"s);
      }
    }
  }

  explicit SearchServer(const std::string &stop_words_text);

  const std::set<std::string> &GetStopWords() const
  {
    return stop_words_;
  }

  void
  AddDocument(int document_id, const std::string &document, DocumentStatus status, const std::vector<int> &ratings);

  template <typename DocumentPredicate>
  std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {
    using namespace std::literals;

    Query query;
    if (!ParseQuery(raw_query, query)) {
      throw std::invalid_argument("Invalid raw_query"s);
    }
    auto matched_documents = FindAllDocuments(query, document_predicate);

    sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
      if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) {
        return lhs.rating > rhs.rating;
      } else {
        return lhs.relevance > rhs.relevance;
      }
    });
    if (matched_documents.size() > MaxResultDocumentCount) {
      matched_documents.resize(MaxResultDocumentCount);
    }

    // Exchange matched_documents and result instead of deep copying
    return matched_documents;
  }

  std::vector<Document> FindTopDocuments(const std::string &raw_query, DocumentStatus status) const;

  std::vector<Document> FindTopDocuments(const std::string &raw_query) const;

  [[nodiscard]]
  const std::map<std::string, double> &GetWordFrequencies(int document_id) const;

  void RemoveDocument(int document_id);

  int GetDocumentCount() const;

  std::vector<int>::iterator begin();

  std::vector<int>::iterator end();

  std::tuple<std::vector<std::string>, DocumentStatus>
  MatchDocument(const std::string &raw_query, int document_id) const;

private:
  struct DocumentData
  {
    int rating;
    DocumentStatus status;
  };

  static const int MaxResultDocumentCount;

  const std::set<std::string> stop_words_;
  std::map<std::string, std::map<int, double>> word_to_document_freqs_;
  std::map<int, DocumentData> documents_;
  std::vector<int> document_ids_;

  static bool IsValidWord(const std::string &word);

  bool IsStopWord(const std::string &word) const;

  std::vector<std::string> SplitIntoWordsNoStop(const std::string &text) const;

  static int ComputeAverageRating(const std::vector<int> &ratings);

  struct QueryWord
  {
    std::string data;
    bool is_minus;
    bool is_stop;
  };

  struct Query
  {
    std::set<std::string> plus_words;
    std::set<std::string> minus_words;
  };

  QueryWord ParseQueryWord(const std::string &text) const;

  bool ParseQueryWord(std::string text, QueryWord &result) const;

  Query ParseQuery(const std::string &text) const;

  bool ParseQuery(const std::string &text, Query &result) const;

  double ComputeWordInverseDocumentFreq(const std::string &word) const;

  template<typename DocumentPredicate>
  std::vector<Document> FindAllDocuments(const Query &query, DocumentPredicate document_predicate) const
  {
    std::map<int, double> document_to_relevance;
    std::set<std::string> words{};

    for (auto &word : query.plus_words)
    {
      if (!stop_words_.count(word))
      {
        words.insert(word);
      }
    }

    for (const std::string &word: words)
    {
      if (word_to_document_freqs_.count(word) == 0)
      {
        continue;
      }
      const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
      for (const auto[document_id, term_freq]: word_to_document_freqs_.at(word))
      {
        const auto &document_data = documents_.at(document_id);
        if (document_predicate(document_id, document_data.status, document_data.rating))
        {
          document_to_relevance[document_id] += term_freq * inverse_document_freq;
        }
      }
    }

    for (const std::string &word: query.minus_words)
    {
      if (word_to_document_freqs_.count(word) == 0)
      {
        continue;
      }
      for (const auto[document_id, _]: word_to_document_freqs_.at(word))
      {
        document_to_relevance.erase(document_id);
      }
    }

    std::vector<Document> matched_documents;
    for (const auto[document_id, relevance]: document_to_relevance)
    {
      matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
  }
};
