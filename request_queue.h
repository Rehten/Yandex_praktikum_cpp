#pragma once

#include <vector>
#include <deque>

#include "search_server.h"

class RequestQueue
{
  public:
    explicit RequestQueue(const SearchServer &search_server);

    template<typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string &raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string &raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string &raw_query);

    int GetNoResultRequests() const;

  private:
    struct QueryResult
    {
        QueryResult(size_t fndd);

        size_t founded;
    };

    const static int sec_in_day_;

    std::deque<QueryResult> requests_;
    const SearchServer &search_server_;

    void UpdateRequests(QueryResult queryResult);
};
