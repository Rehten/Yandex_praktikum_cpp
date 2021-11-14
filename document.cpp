#include "document.h"

using std::ostream;
using std::vector;

Document::Document(): Document(0, 0, 0)
{}

Document::Document(int id, double relevance, int rating)
  : id(id), relevance(relevance), rating(rating)
{}

ostream &operator << (ostream &os, vector<Document> page)
{
  for (const auto &document : page)
  {
    os << "{ document_id = " << document.id << ", relevance = " << document.relevance << ", rating = " << document.rating << " }";
  }

  return os;
}

