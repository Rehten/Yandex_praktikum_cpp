#pragma once

#include <iostream>
#include <vector>

enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document
{
    Document();

    Document(int id, double relevance, int rating);

    int id;
    double relevance;
    int rating;
};

std::ostream &operator<<(std::ostream &os, std::vector<Document> page);
