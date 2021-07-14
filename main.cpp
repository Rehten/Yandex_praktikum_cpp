

using namespace std;

const int EPSILON = 1e-6;

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
  const int doc_id = 42;
  const string content = "cat in the city"s;
  const vector<int> ratings = {1, 2, 3};
  // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
  // находит нужный документ
  {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    const auto found_docs = server.FindTopDocuments("in"s);
    ASSERT(found_docs.size() == 1);
    const Document& doc0 = found_docs[0];
    ASSERT(doc0.id == doc_id);
  }

  // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
  // возвращает пустой результат
  {
    SearchServer server;
    server.SetStopWords("in the"s);
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    ASSERT(server.FindTopDocuments("in"s).empty());
  }
}

/*
Разместите код остальных тестов здесь
*/
void TestRemovedDocumentIfMatchMinusWord() {
  const int doc_id = 42;
  const string content = "cat in the city"s;
  const vector<int> ratings = {1, 2, 3};

  {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    const auto found_docs = server.FindTopDocuments("-in"s);
    ASSERT(!found_docs.size());
  }
}
void TestMatchedWords() {
  const int doc_id = 42;
  const string content = "cat in the city"s;
  const string content1 = "White meat on black street"s;
  const vector<int> ratings = {1, 2, 3};
  const vector<int> ratings1 = {2, 3, 4};

  {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id + 1, content1, DocumentStatus::ACTUAL, ratings1);
    const auto found_docs = server.FindTopDocuments("in meat"s);
    ASSERT(found_docs[0].relevance);
    ASSERT(found_docs[1].relevance);
  }
}

void TestRatingWords() {
  const int doc_id = 42;
  const string content = "cat in the city"s;
  const string content1 = "White meat on black street"s;
  const vector<int> ratings = {1, 2, 3};
  const vector<int> ratings1 = {2, 3, 4};

  {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id + 1, content1, DocumentStatus::ACTUAL, ratings1);
    const auto found_docs = server.FindTopDocuments("in meat"s);
    ASSERT(found_docs[0].rating == 2);
    ASSERT(found_docs[1].rating == 3);
  }
}

void TestSortedByRelevanceWords() {
  const int doc_id = 42;
  const string content = "cat in the city"s;
  const string content1 = "White meat on black street"s;
  const string content2 = "in in in in"s;
  const string content3 = "in in in in"s;
  const vector<int> ratings = {1, 2, 3};
  const vector<int> ratings1 = {2, 3, 4};
  const vector<int> ratings2 = {2, 3, 4};
  const vector<int> ratings3 = {6, 6, 6};

  {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id + 1, content1, DocumentStatus::ACTUAL, ratings1);
    server.AddDocument(doc_id + 2, content2, DocumentStatus::ACTUAL, ratings2);
    server.AddDocument(doc_id + 3, content3, DocumentStatus::ACTUAL, ratings3);
    const auto found_docs = server.FindTopDocuments("in meat"s);

    ASSERT(found_docs[0].id == 45);
    ASSERT(found_docs[1].id == 44);
    ASSERT(found_docs[2].id == 43);
    ASSERT(found_docs[3].id == 42);
  }
}

void TestFilteredByUserWords() {
  const int doc_id = 42;
  const string content = "cat in the city"s;
  const string content1 = "White meat on black street"s;
  const string content2 = "in in in in"s;
  const vector<int> ratings = {1, 2, 3};
  const vector<int> ratings1 = {2, 3, 4};
  const vector<int> ratings2 = {2, 3, 4};

  {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id + 1, content1, DocumentStatus::IRRELEVANT, ratings1);
    server.AddDocument(doc_id + 2, content2, DocumentStatus::BANNED, ratings2);
    const auto found_docs = server.FindTopDocuments(
      "in meat"s,
      [](int document_id, DocumentStatus status, int rating) -> bool {
        return status == DocumentStatus::ACTUAL || status == DocumentStatus::BANNED;
      }
    );
    ASSERT(found_docs.size() == 2);
    ASSERT(found_docs[0].id == 44);
    ASSERT(found_docs[1].id == 42);
  }
}

void TestCalculatedRelevance() {

  const int doc_id = 42;
  const string content = "cat in the city"s;
  const string content1 = "White meat on black street"s;
  const string content2 = "in in in in"s;
  const vector<int> ratings = {1, 2, 3};
  const vector<int> ratings1 = {2, 3, 4};
  const vector<int> ratings2 = {2, 3, 4};

  {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id + 1, content1, DocumentStatus::ACTUAL, ratings1);
    server.AddDocument(doc_id + 2, content2, DocumentStatus::ACTUAL, ratings2);
    const auto found_docs = server.FindTopDocuments("in meat"s);
    ASSERT(abs(found_docs[0].relevance - 0.405465) > EPSILON);
    ASSERT(abs(found_docs[1].relevance - 0.219722) > EPSILON);
    ASSERT(abs(found_docs[2].relevance - 0.101366) > EPSILON);
  }
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
  TestExcludeStopWordsFromAddedDocumentContent();
  TestRemovedDocumentIfMatchMinusWord();
  TestMatchedWords();
  TestRatingWords();
  TestSortedByRelevanceWords();
  TestFilteredByUserWords();
  TestCalculatedRelevance();
}
