#pragma once

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <execution>

#include "read_input_functions.h"
#include "string_processing.h"
#include "paginator.h"
#include "log_duration.h"

class SearchServer
{
  public:
    static const int MaxResultDocumentCount;

    template<typename StringContainer>
    SearchServer(const StringContainer &stop_words)
      : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
    {
      using namespace std;
      using namespace literals;
      for (auto &word: stop_words_)
      {
        if (!IsValidWord({word.begin(),  word.end()}))
        {
          throw invalid_argument("Word "s + word + "is invalid"s);
        }
      }
    }

    const std::set<std::string> &GetStopWords() const
    {
      return stop_words_;
    }

    explicit SearchServer(const std::string_view &stop_words_text);
    explicit SearchServer(const std::string &stop_words_text);

    void
    AddDocument(int document_id, const std::string_view &document, DocumentStatus status, const std::vector<int> &ratings);

    template<typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view &raw_query, DocumentPredicate document_predicate) const
    {
      using namespace std::literals;

      Query query;
      if (!ParseQuery({raw_query.begin(),  raw_query.end()}, query))
      {
        throw std::invalid_argument("Invalid raw_query"s);
      }
      auto matched_documents = FindAllDocuments(query, document_predicate);

      sort(std::execution::par, matched_documents.begin(), matched_documents.end(), [](const Document &lhs, const Document &rhs) {
        if (std::abs(lhs.relevance - rhs.relevance) < 1e-6)
        {
          return lhs.rating > rhs.rating;
        }
        else
        {
          return lhs.relevance > rhs.relevance;
        }
      });
      if (matched_documents.size() > MaxResultDocumentCount)
      {
        matched_documents.resize(MaxResultDocumentCount);
      }

      // Exchange matched_documents and result instead of deep copying
      return matched_documents;
    }

    std::vector<Document> FindTopDocuments(const std::string_view &raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(const std::string_view &raw_query) const;

    [[nodiscard]]
    const std::map<std::string_view, double> &GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);

    template<class Policy>
    void RemoveDocument(Policy exec_strategy, int document_id)
    {
      document_ids_.erase(find(document_ids_.begin(), document_ids_.end(), document_id));
      documents_.erase(document_id);

      std::vector<std::pair<std::string, std::map<int, double>>> word_to_document_freqs {word_to_document_freqs_.begin(),  word_to_document_freqs_.end()};

      std::for_each(
        exec_strategy,
        word_to_document_freqs.begin(),
        word_to_document_freqs.end(),
        [document_id](std::pair<std::string, std::map<int, double>> &pair) -> void
        {
          if (pair.second.count(document_id))
          {
            pair.second.erase(document_id);
          }
        });
    }

    int GetDocumentCount() const;

    std::vector<int>::iterator begin();

    std::vector<int>::iterator end();

    std::tuple<std::vector<std::string_view>, DocumentStatus>
    MatchDocument(const std::string_view &raw_query, int document_id) const;

    template<class ExectutionStrategy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExectutionStrategy strategy, const std::string_view &raw_query, int document_id) const
    {
      // Empty result by initializing it with default constructed tuple
      using namespace std::literals;

      Query query;
      if (!ParseQuery(std::string{raw_query.begin(),  raw_query.end()}, query))
      {
        throw std::invalid_argument("Invalid raw query!"s);
      }

      if (
        std::find_if(strategy, query.minus_words.begin(),  query.minus_words.end(), [this, document_id](const std::string &word) -> bool
        {
          return word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id);
        }) != query.minus_words.end()
        )
      {
        return {{}, documents_.at(document_id).status};
      }

      std::vector<std::string_view> matched_words;
      matched_words.reserve(query.plus_words.size());

      std::for_each(strategy, query.plus_words.begin(),  query.plus_words.end(), [this, &matched_words, document_id](const std::string &word) -> void
      {
        if (word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id))
        {
          if (!saved_documents_.count(word))
          {
            auto str = std::string{word.begin(),  word.end()};
            saved_documents_[str] = str;
          }

          matched_words.push_back(word);
        }
      });

      return {matched_words, documents_.at(document_id).status};
    }

  private:
    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    mutable std::map<std::string, std::string> saved_documents_;
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

      for (auto &word: query.plus_words)
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
