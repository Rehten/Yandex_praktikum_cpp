#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
 public:
  using runtime_error::runtime_error;
};

class Node {
 public:
  using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
  /* Реализуйте Node, используя std::variant */

  explicit Node(Array array);
  explicit Node(Dict map);
  explicit Node(int value);

  explicit Node(std::string value);
  const Array& AsArray() const;
  const Dict& AsMap() const;
  int AsInt() const;

  const std::string& AsString() const;

  const Value &GetValue() const {
	return value_;
  }
 private:
  Value value_ = std::nullptr_t();
};

class Document {
 public:
  explicit Document(Node root);

  const Node& GetRoot() const;

 private:
  Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json