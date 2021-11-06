#include <cmath>
#include <numeric>
#include <execution>

#include "process_queries.h"

using namespace std;

vector<vector<Document>> ProcessQueries(
  const SearchServer &search_server,
  const vector<string> &queries
)
{
  vector<vector<Document>> rslt{queries.size()};

  transform(
    execution::par,
    queries.begin(),
    queries.end(),
    rslt.begin(),
    [&search_server](const string &query) -> vector<Document> {
      return search_server.FindTopDocuments(query);
    }
  );

  return rslt;
}

vector<Document> ProcessQueriesJoined(
  const SearchServer &search_server,
  const vector<string> &queries
)
{
  vector<vector<Document>> response = ProcessQueries(search_server, queries);
  auto rslt = vector<Document>{};
  size_t accum_size {};

  for (auto &v : response)
  {
    accum_size += v.size();
  }

  rslt.reserve(accum_size);

  return reduce(
    response.begin(),
    response.end(),
    rslt,
    [](vector<Document> &acc, vector<Document> &cur)
    {
      for (const Document &doc : cur)
      {
        acc.push_back(doc);
      }

      return acc;
    }
  );
}
