// search_server_s3_t3_v1.cpp

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

string ReadLine() {
  string s;
  getline(cin, s);
  return s;
}

int ReadLineWithNumber() {
  int result;
  cin >> result;
  ReadLine();
  return result;
}

vector<string> SplitIntoWords(const string& text) {
  vector<string> words;
  string word;
  for (const char c : text) {
    if (c == ' ') {
      if (!word.empty()) {
        words.push_back(word);
        word.clear();
      }
    } else {
      word += c;
    }
  }
  if (!word.empty()) {
    words.push_back(word);
  }

  return words;
}

struct Document {
  Document() = default;

  Document(int id, double relevance, int rating)
    : id(id)
    , relevance(relevance)
    , rating(rating) {
  }

  int id = 0;
  double relevance = 0.0;
  int rating = 0;
};

template <typename StringContainer>
set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
  set<string> non_empty_strings;
  for (const string& str : strings) {
    if (!str.empty()) {
      non_empty_strings.insert(str);
    }
  }
  return non_empty_strings;
}

enum class DocumentStatus {
  ACTUAL,
  IRRELEVANT,
  BANNED,
  REMOVED,
};

class SearchServer {
public:
  // Defines an invalid document id
  // You can refer this constant as SearchServer::INVALID_DOCUMENT_ID
  inline static constexpr int INVALID_DOCUMENT_ID = -1;

  template <typename StringContainer>
  explicit SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
      for (auto & word : stop_words_)
      {
        if (!IsValidWord(word))
        {
          throw invalid_argument("Word "s + word + "is invalid"s);
        }
      }
  }

  explicit SearchServer(const string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))  // Invoke delegating constructor from string container
  {
  }

  void AddDocument(int document_id, const string& document, DocumentStatus status,
                                 const vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
      throw invalid_argument("Invalid document arguments");
    }
    vector<string> words;
    if (!SplitIntoWordsNoStop(document, words)) {
      throw invalid_argument("Invalid document arguments");
    }

    const double inv_word_count = 1.0 / words.size();
    for (const string& word : words) {
      word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.push_back(document_id);
  }

  template <typename DocumentPredicate>
  vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const {
    Query query;
    if (!ParseQuery(raw_query, query)) {
      throw invalid_argument("Invalid raw_query"s);
    }
    auto matched_documents = FindAllDocuments(query, document_predicate);

    sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
      if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
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

  vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(
      raw_query,
      [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
      });
  }

  vector<Document> FindTopDocuments(const string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
  }

  int GetDocumentCount() const {
    return documents_.size();
  }

  int GetDocumentId(int index) const {
    if (!(index >= 0 && index < GetDocumentCount())) {
      throw out_of_range("Not found document");
    }
    return document_ids_[index];
  }

  tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
    // Empty result by initializing it with default constructed tuple
    Query query;
    if (!ParseQuery(raw_query, query)) {
      throw invalid_argument("Invalid raw query!"s);
    }
    vector<string> matched_words;
    for (const string& word : query.plus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      if (word_to_document_freqs_.at(word).count(document_id)) {
        matched_words.push_back(word);
      }
    }
    for (const string& word : query.minus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      if (word_to_document_freqs_.at(word).count(document_id)) {
        matched_words.clear();
        break;
      }
    }

    return {matched_words, documents_.at(document_id).status};
  }

private:
  struct DocumentData {
    int rating;
    DocumentStatus status;
  };
  static const int MaxResultDocumentCount = 5;

  const set<string> stop_words_;
  map<string, map<int, double>> word_to_document_freqs_;
  map<int, DocumentData> documents_;
  vector<int> document_ids_;

  bool IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
  }

  static bool IsValidWord(const string& word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
      return c >= '\0' && c < ' ';
    });
  }

  bool SplitIntoWordsNoStop(const string& text, vector<string>& result) const {
    result.clear();
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
      if (!IsValidWord(word)) {
        return false;
      }
      if (!IsStopWord(word)) {
        words.push_back(word);
      }
    }
    result.swap(words);
    return true;
  }

  static int ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
      return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
      rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
  }

  struct QueryWord {
    string data;
    bool is_minus;
    bool is_stop;
  };

  bool ParseQueryWord(string text, QueryWord& result) const {
    // Empty result by initializing it with default constructed QueryWord
    result = {};

    if (text.empty()) {
      return false;
    }
    bool is_minus = false;
    if (text[0] == '-') {
      is_minus = true;
      text = text.substr(1);
    }
    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
      return false;
    }

    result = QueryWord{text, is_minus, IsStopWord(text)};
    return true;
  }

  struct Query {
    set<string> plus_words;
    set<string> minus_words;
  };

  bool ParseQuery(const string& text, Query& result) const {
    // Empty result by initializing it with default constructed Query
    result = {};
    for (const string& word : SplitIntoWords(text)) {
      QueryWord query_word;
      if (!ParseQueryWord(word, query_word)) {
        return false;
      }
      if (!query_word.is_stop) {
        if (query_word.is_minus) {
          result.minus_words.insert(query_word.data);
        } else {
          result.plus_words.insert(query_word.data);
        }
      }
    }
    return true;
  }

  // Existence required
  double ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
  }

  template <typename DocumentPredicate>
  vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
    map<int, double> document_to_relevance;
    for (const string& word : query.plus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
      for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
        const auto& document_data = documents_.at(document_id);
        if (document_predicate(document_id, document_data.status, document_data.rating)) {
          document_to_relevance[document_id] += term_freq * inverse_document_freq;
        }
      }
    }

    for (const string& word : query.minus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
        document_to_relevance.erase(document_id);
      }
    }

    vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
      matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
  }
};


int main() {
  SearchServer search_server("и в на"s);

  AddDocument(search_server, 1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
  AddDocument(search_server, 1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2});
  AddDocument(search_server, -1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2});
  AddDocument(search_server, 3, "большой пёс скво\x12рец евгений"s, DocumentStatus::ACTUAL, {1, 3, 2});
  AddDocument(search_server, 4, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, {1, 1, 1});

  FindTopDocuments(search_server, "пушистый -пёс"s);
  FindTopDocuments(search_server, "пушистый --кот"s);
  FindTopDocuments(search_server, "пушистый -"s);

  MatchDocuments(search_server, "пушистый пёс"s);
  MatchDocuments(search_server, "модный -кот"s);
  MatchDocuments(search_server, "модный --пёс"s);
  MatchDocuments(search_server, "пушистый - хвост"s);
}
