#include "remove_duplicates.h"

#include <iostream>
#include <map>
#include <algorithm>

using namespace std;

void RemoveDuplicates(SearchServer &search_server)
{
  set<map<string, double>> document_ids_to_word_freqs{};
  vector<int> ids_to_delete{};

  ids_to_delete.reserve(search_server.GetDocumentCount());

  for (int document_id: search_server)
  {
    auto word_freqs = search_server.GetWordFrequencies(document_id);

    for (auto &word : search_server.GetStopWords())
    {
      if (word_freqs.count(word))
      {
        word_freqs.erase(word);
      }
    }

    for (auto &[str, freq] : word_freqs)
    {
      freq = 0;
    }

    if (document_ids_to_word_freqs.count(word_freqs))
    {
      cout << "Found duplicate document id "s << document_id << endl;
      ids_to_delete.push_back(document_id);
    }
    else
    {
      document_ids_to_word_freqs.insert(word_freqs);
    }
  }

  for (int document_id: ids_to_delete)
  {
    search_server.RemoveDocument(document_id);
  }
}
