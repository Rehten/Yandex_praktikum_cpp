// search_server_s4_t2_solution.cpp

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <deque>

#include "document.h"

using namespace std;

string ReadLine()
{
  string s;
  getline(cin, s);
  return s;
}

int ReadLineWithNumber()
{
  int result;
  cin >> result;
  ReadLine();
  return result;
}

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

enum class DocumentStatus
  {
        ACTUAL,
        IRRELEVANT,
        BANNED,
        REMOVED,
        };

class SearchServer
  {
  public:
    template<typename StringContainer>
    explicit SearchServer(const StringContainer &stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  // Extract non-empty stop words
    {
      if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord))
      {
        throw invalid_argument("Some of stop words are invalid"s);
      }
    }

    explicit SearchServer(const string &stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))  // Invoke delegating constructor from string container
    {
    }

    void AddDocument(int document_id, const string &document, DocumentStatus status, const vector<int> &ratings)
    {
      if ((document_id < 0) || (documents_.count(document_id) > 0))
      {
        throw invalid_argument("Invalid document_id"s);
      }
      const auto words = SplitIntoWordsNoStop(document);

      const double inv_word_count = 1.0 / words.size();
      for (const string &word : words)
      {
        word_to_document_freqs_[word][document_id] += inv_word_count;
      }
      documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
      document_ids_.push_back(document_id);
    }

    template<typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string &raw_query, DocumentPredicate document_predicate) const
    {
      const auto query = ParseQuery(raw_query);

      auto matched_documents = FindAllDocuments(query, document_predicate);

      sort(matched_documents.begin(), matched_documents.end(), [](const Document &lhs, const Document &rhs) {
        if (abs(lhs.relevance - rhs.relevance) < 1e-6)
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

      return matched_documents;
    }

    vector<Document> FindTopDocuments(const string &raw_query, DocumentStatus status) const
    {
      return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
      });
    }

    vector<Document> FindTopDocuments(const string &raw_query) const
    {
      return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    int GetDocumentCount() const
    {
      return documents_.size();
    }

    int GetDocumentId(int index) const
    {
      return document_ids_.at(index);
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string &raw_query, int document_id) const
    {
      const auto query = ParseQuery(raw_query);

      vector<string> matched_words;
      for (const string &word : query.plus_words)
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
      for (const string &word : query.minus_words)
      {
        if (word_to_document_freqs_.count(word) == 0)
        {
          continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id))
        {
          matched_words.clear();
          break;
        }
      }
      return {matched_words, documents_.at(document_id).status};
    }

  private:
    struct DocumentData
      {
        int rating;
        DocumentStatus status;
      };

    static const int MaxResultDocumentCount = 5;

    const set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> document_ids_;

    bool IsStopWord(const string &word) const
    {
      return stop_words_.count(word) > 0;
    }

    static bool IsValidWord(const string &word)
    {
      // A valid word must not contain special characters
      return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
      });
    }

    vector<string> SplitIntoWordsNoStop(const string &text) const
    {
      vector<string> words;
      for (const string &word : SplitIntoWords(text))
      {
        if (!IsValidWord(word))
        {
          throw invalid_argument("Word "s + word + " is invalid"s);
        }
        if (!IsStopWord(word))
        {
          words.push_back(word);
        }
      }
      return words;
    }

    static int ComputeAverageRating(const vector<int> &ratings)
    {
      if (ratings.empty())
      {
        return 0;
      }
      int rating_sum = 0;
      for (const int rating : ratings)
      {
        rating_sum += rating;
      }
      return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord
      {
      string data;
      bool is_minus;
      bool is_stop;
      };

    QueryWord ParseQueryWord(const string &text) const
    {
      if (text.empty())
      {
        throw invalid_argument("Query word is empty"s);
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
        throw invalid_argument("Query word "s + text + " is invalid");
      }

      return {word, is_minus, IsStopWord(word)};
    }

    struct Query
      {
      set<string> plus_words;
      set<string> minus_words;
      };

    Query ParseQuery(const string &text) const
    {
      Query result;
      for (const string &word : SplitIntoWords(text))
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

    // Existence required
    double ComputeWordInverseDocumentFreq(const string &word) const
    {
      return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template<typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query &query, DocumentPredicate document_predicate) const
    {
      map<int, double> document_to_relevance;
      for (const string &word : query.plus_words)
      {
        if (word_to_document_freqs_.count(word) == 0)
        {
          continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto[document_id, term_freq] : word_to_document_freqs_.at(word))
        {
          const auto &document_data = documents_.at(document_id);
          if (document_predicate(document_id, document_data.status, document_data.rating))
          {
            document_to_relevance[document_id] += term_freq * inverse_document_freq;
          }
        }
      }

      for (const string &word : query.minus_words)
      {
        if (word_to_document_freqs_.count(word) == 0)
        {
          continue;
        }
        for (const auto[document_id, _] : word_to_document_freqs_.at(word))
        {
          document_to_relevance.erase(document_id);
        }
      }

      vector<Document> matched_documents;
      for (const auto[document_id, relevance] : document_to_relevance)
      {
        matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
      }
      return matched_documents;
    }
  };

class Paginator : public vector<vector<Document>>
  {
  private:
    size_t page_size_;
  public:
    template<typename I>
    Paginator(
      I start,
      I end,
      size_t page_size
      )
      :
      page_size_(page_size)
      {
        int i {};
        vector<Document> v {};

        for (I iter = start; iter != end; ++iter)
        {
          ++i;
          v.push_back(*iter);

          if (i == page_size_)
          {
            push_back({v.begin(), v.end()});
            v.clear();
            i = 0;
          }
        }
      }
  };

template<typename Container>
auto Paginate(const Container &c, size_t page_size)
{
  return Paginator(c.begin(), c.end(), page_size);
}


ostream &operator << (ostream &os, vector<Document> page)
{
  for (const auto &document : page)
  {
    os << "{ document_id = " << document.id << ", relevance = " << document.relevance << ", rating = " << document.rating << " }";
  }

  return os;
}


class RequestQueue
  {
  public:
    explicit RequestQueue(const SearchServer &search_server) : search_server_(search_server)
    {}

    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template<typename DocumentPredicate>
    vector<Document> AddFindRequest(const string &raw_query, DocumentPredicate document_predicate)
    {
      vector<Document> response = search_server_.FindTopDocuments(raw_query, document_predicate);

      UpdateRequests({response.size()});

      return response;
    }

    vector<Document> AddFindRequest(const string &raw_query, DocumentStatus status)
    {
      vector<Document> response = search_server_.FindTopDocuments(raw_query, status);

      UpdateRequests({response.size()});

      return response;
    }

    vector<Document> AddFindRequest(const string &raw_query)
    {
      vector<Document> response = search_server_.FindTopDocuments(raw_query);

      UpdateRequests({response.size()});

      return response;
    }

    int GetNoResultRequests() const
    {

      return count_if(requests_.rbegin(), requests_.rend(), [](QueryResult rslt) -> bool {
        return !rslt.founded;
      });
    }

  private:
    struct QueryResult
      {
      // определите, что должно быть в структуре
      QueryResult(size_t fndd): founded(fndd)
      {}
      size_t founded;
      };
    deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    // возможно, здесь вам понадобится что-то ещё
    const SearchServer &search_server_;

    void UpdateRequests(QueryResult queryResult)
    {
      requests_.push_back(queryResult);

      if (requests_.size() > sec_in_day_)
      {
        requests_.pop_front();
      }
    }
  };

int main() {
  SearchServer search_server("and in at"s);
  RequestQueue request_queue(search_server);

  search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
  search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
  search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
  search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
  search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});

  // 1439 запросов с нулевым результатом
  for (int i = 0; i < 1439; ++i) {
    request_queue.AddFindRequest("empty response"s);
  }
  // все еще 1439 запросов с нулевым результатом
  request_queue.AddFindRequest("curly dog"s);
  // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
  request_queue.AddFindRequest("big collar"s);
  // первый запрос удален, 1437 запросов с нулевым результатом
  request_queue.AddFindRequest("sparrow"s);
  cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
  return 0;
}
