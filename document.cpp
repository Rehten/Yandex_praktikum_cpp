#pragma once
#include "document.h"

Document::Document(): Document(0, 0, 0)
{}

Document::Document(int id, double relevance, int rating)
  : id(id), relevance(relevance), rating(rating)
{}
