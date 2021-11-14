#include <algorithm>

#include "request_queue.h"

using std::vector;
using std::string;

RequestQueue::RequestQueue(const SearchServer &search_server) : search_server_(search_server)
{}

template<typename DocumentPredicate>
vector<Document> RequestQueue::AddFindRequest(const string &raw_query, DocumentPredicate document_predicate)
{
  vector<Document> response = search_server_.FindTopDocuments(raw_query, document_predicate);

  UpdateRequests({response.size()});

  return response;
}

vector<Document> RequestQueue::AddFindRequest(const string &raw_query, DocumentStatus status)
{
  vector<Document> response = search_server_.FindTopDocuments(raw_query, status);

  UpdateRequests({response.size()});

  return response;
}

vector<Document> RequestQueue::AddFindRequest(const string &raw_query)
{
  vector<Document> response = search_server_.FindTopDocuments(raw_query);

  UpdateRequests({response.size()});

  return response;
}

int RequestQueue::GetNoResultRequests() const
{
  return count_if(requests_.rbegin(), requests_.rend(), [](QueryResult rslt) -> bool {
    return !rslt.founded;
  });
}

const int RequestQueue::sec_in_day_ = 1440;

void RequestQueue::UpdateRequests(QueryResult queryResult)
{
  requests_.push_back(queryResult);

  if (requests_.size() > sec_in_day_)
  {
    requests_.pop_front();
  }
}

RequestQueue::QueryResult::QueryResult(size_t fndd) : founded(fndd)
{}
