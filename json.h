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

  Node();
  Node(Value value) noexcept;

  bool IsNull() const;
  bool IsInt() const;
  bool IsBool() const;
  bool IsDouble() const;
  bool IsPureDouble() const;
  bool IsString() const;
  bool IsArray() const;
  bool IsMap() const;

  const std::nullptr_t &AsNull() const;
  int AsInt() const;
  int AsBool() const;
  double AsDouble() const;
  double AsPureDouble() const;
  const std::string &AsString() const;
  const Array &AsArray() const;
  const Dict &AsMap() const;

  Node &operator=(const Value &);
  Node &operator=(Value &&);

  bool operator==(const Node &) const;
  bool operator!=(const Node &) const;

  [[nodiscard]] const Value &GetValue() const {
	return value_;
  }
 private:
  void Swap(Node &, Node &) noexcept;
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

enum class NodeType {
  NULLPTR,
  ARRAY,
  DICT,
  BOOL,
  INT,
  DOUBLE,
  STRING,
};

class NodeTypeChecker {
 public:
  NodeType operator()(const std::nullptr_t &);
  NodeType operator()(const Array &);
  NodeType operator()(const Dict &);
  NodeType operator()(const bool &);
  NodeType operator()(const int &);
  NodeType operator()(const double &);
  NodeType operator()(const std::string &);
};

class NodeStringifier {
 public:
  std::string operator()(const std::nullptr_t &);
  std::string operator()(const Array &);
  std::string operator()(const Dict &);
  std::string operator()(const bool &);
  std::string operator()(const int &);
  std::string operator()(const double &);
  std::string operator()(const std::string &);
};

}  // namespace json