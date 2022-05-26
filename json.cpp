#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
  Array result;

  for (char c; input >> c && c != ']';) {
	if (c != ',') {
	  input.putback(c);
	}
	result.push_back(LoadNode(input));
  }

  return Node(move(result));
}

Node LoadInt(istream& input) {
  int result = 0;
  while (isdigit(input.peek())) {
	result *= 10;
	result += input.get() - '0';
  }
  return Node(result);
}

Node LoadString(istream& input) {
  string line;
  getline(input, line, '"');
  return Node(move(line));
}

Node LoadDict(istream& input) {
  Dict result;

  for (char c; input >> c && c != '}';) {
	if (c == ',') {
	  input >> c;
	}

	string key = LoadString(input).AsString();
	input >> c;
	result.insert({move(key), LoadNode(input)});
  }

  return Node(move(result));
}

Node LoadNode(istream& input) {
  char c;
  input >> c;

  if (c == '[') {
	return LoadArray(input);
  } else if (c == '{') {
	return LoadDict(input);
  } else if (c == '"') {
	return LoadString(input);
  } else {
	input.putback(c);
	return LoadInt(input);
  }
}

}  // namespace
Node::Node() : Node(nullptr) {}
Node::Node(double value) noexcept : value_(value) {}
Node::Node(int value) noexcept : value_(value) {}
Node::Node(bool value) noexcept : value_(value) {}
Node::Node(nullptr_t value) noexcept : value_(value) {}
Node::Node(Array value) noexcept : value_(move(value)) {}
Node::Node(Dict value) noexcept : value_(move(value)) {}
Node::Node(string value) noexcept : value_(move(value)) {}

void Node::Swap(Node &lhs, Node &rhs) noexcept {
  swap<Value>(lhs.value_, rhs.value_);
}
Node &Node::operator=(const Value &rhs) {
  this->value_ = rhs;

  return *this;
}
Node &Node::operator=(Value &&rhs) {
  *this = rhs;

  return *this;
}
bool Node::IsNull() const {
  return visit(NodeTypeChecker(), value_) == NodeType::NULLPTR;
}
bool Node::IsInt() const {
  return visit(NodeTypeChecker(), value_) == NodeType::INT;
}
bool Node::IsBool() const {
  return visit(NodeTypeChecker(), value_) == NodeType::BOOL;
}
bool Node::IsDouble() const {
  return IsInt() || IsPureDouble();
}
bool Node::IsPureDouble() const {
  return visit(NodeTypeChecker(), value_) == NodeType::DOUBLE;
}
bool Node::IsString() const {
  return visit(NodeTypeChecker(), value_) == NodeType::STRING;
}
bool Node::IsArray() const {
  return visit(NodeTypeChecker(), value_) == NodeType::ARRAY;
}
bool Node::IsMap() const {
  return visit(NodeTypeChecker(), value_) == NodeType::DICT;
}

const nullptr_t &Node::AsNull() const {
  return get<nullptr_t>(value_);
}
int Node::AsInt() const {
  return get<int>(value_);
}
int Node::AsBool() const {
  return get<bool>(value_);
}
double Node::AsDouble() const {
  return get<double>(value_);
}
double Node::AsPureDouble() const {
  return get<double>(value_);
}
const string &Node::AsString() const {
  return get<string>(value_);
}
const Array &Node::AsArray() const {
  return get<Array>(value_);
}
const Dict &Node::AsMap() const {
  return get<Dict>(value_);
}
bool Node::operator==(const Node &rhs) const {
  if (IsInt()) {
	return get<int>(GetValue()) == get<int>(rhs.GetValue());
  } else if (IsPureDouble()) {
	return get<double>(GetValue()) == get<double>(rhs.GetValue());
  } else if (IsBool()) {
	return get<bool>(GetValue()) == get<bool>(rhs.GetValue());
  } else if (IsDouble()) {
	return get<double>(GetValue()) == get<double>(rhs.GetValue());
  } else if (IsArray()) {
	return get<Array>(GetValue()) == get<Array>(rhs.GetValue());
  } else if (IsMap()) {
	return get<Dict>(GetValue()) == get<Dict>(rhs.GetValue());
  } else if (IsNull()) {
	return get<nullptr_t>(GetValue()) == get<nullptr_t>(rhs.GetValue());
  }

  return false;
}
bool Node::operator!=(const Node &rhs) const {
  return !this->operator==(rhs);
}

Document::Document(Node root)
	: root_(move(root)) {
}

const Node& Document::GetRoot() const {
  return root_;
}

Document Load(istream& input) {
  return Document{LoadNode(input)};
}

void Print(const Document& doc, ostream& output) {
  output << visit(NodeStringifier(), doc.GetRoot().GetValue());
}

NodeType NodeTypeChecker::operator()(const nullptr_t &) {
  return NodeType::NULLPTR;
}
NodeType NodeTypeChecker::operator()(const Array &) {
  return NodeType::ARRAY;
}
NodeType NodeTypeChecker::operator()(const Dict &) {
  return NodeType::DICT;
}
NodeType NodeTypeChecker::operator()(const bool &) {
  return NodeType::BOOL;
}
NodeType NodeTypeChecker::operator()(const int &) {
  return NodeType::INT;
}
NodeType NodeTypeChecker::operator()(const double &) {
  return NodeType::DOUBLE;
}
NodeType NodeTypeChecker::operator()(const string &) {
  return NodeType::STRING;
}
string NodeStringifier::operator()(const nullptr_t &) {
  return "null"s;
}
string NodeStringifier::operator()(const Array &array) {
  const static size_t NODE_LENGTH = 4;
  string rslt{};
  rslt.reserve(array.size() * NODE_LENGTH);

  rslt += "[ ";

  for (size_t i = 0; i != array.size(); ++i) {
	if (i) rslt += ", ";
	rslt += visit(*this, array[i].GetValue());
  }

  rslt += " ]";

  return rslt;
}
string NodeStringifier::operator()(const Dict &obj) {
  string rslt{};
  rslt.reserve(obj.size() * 8);

  rslt += "{ ";
  for (auto iter = obj.begin(); iter !=  obj.end(); ++iter) {
	if (iter != obj.begin()) {
	  rslt += ", "s;
	}
	rslt += "\""s + iter->first + "\": "s + visit(*this, iter->second.GetValue());
  }
  rslt += " }";

  return rslt;
}
string NodeStringifier::operator()(const bool &bool_val) {
  return bool_val ? "true"s : "false"s;
}
string NodeStringifier::operator()(const int &int_val) {
  return to_string(int_val);
}
string NodeStringifier::operator()(const double &double_val) {
  return to_string(double_val);
}
string NodeStringifier::operator()(const string &str_val) {
  return str_val;
}
}  // namespace json