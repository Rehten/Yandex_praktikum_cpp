#include <numeric>
#include <execution>

#include "process_queries.h"

using namespace std;

vector<vector<Document>> ProcessQueries(
  const SearchServer &search_server,
  const vector<string> &queries
)
{
  vector<vector<Document>> rslt {queries.size()};

  transform(
      execution::par,
      queries.begin(),
      queries.end(),
      rslt.begin(),
      [&search_server](const string &query) -> vector<Document>
      {
        return search_server.FindTopDocuments(query);
      }
    );

  return rslt;
}
