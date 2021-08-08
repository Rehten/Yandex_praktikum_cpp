#pragma once

#include <vector>
#include <set>
#include <map>
#include <string>

#include "read_input_functions.h"
#include "string_processing.h"
#include "paginator.h"

class SearchServer
{
public:
  template<typename StringContainer>
  explicit SearchServer(const StringContainer &stop_words);

  explicit SearchServer(const std::string &stop_words_text);

  void
  AddDocument(int document_id, const std::string &document, DocumentStatus status, const std::vector<int> &ratings);

  template<typename DocumentPredicate>
  std::vector<Document> FindTopDocuments(const std::string &raw_query, DocumentPredicate document_predicate) const;

  std::vector<Document> FindTopDocuments(const std::string &raw_query, DocumentStatus status) const;

  std::vector<Document> FindTopDocuments(const std::string &raw_query) const;

  int GetDocumentCount() const;

  int GetDocumentId(int index) const;

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

  Query ParseQuery(const std::string &text) const;

  double ComputeWordInverseDocumentFreq(const std::string &word) const;

  template<typename DocumentPredicate>
  std::vector<Document> FindAllDocuments(const Query &query, DocumentPredicate document_predicate) const;
};
