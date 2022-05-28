#include <sstream>
#include <string_view>
#include <stack>
#include <unordered_map>

#include "json.h"

using namespace std;

json::Node LoadNode(istream &input);

static unordered_map<char, char> open_to_close_bracket = {
  {'{', '}'},
  {'[', ']'},
  {'"', '"'},
};

class ParsingError : public runtime_error {
 public:
  using runtime_error::runtime_error;
};

using Number = variant<int, double>;

bool IsEscapedChar(char c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

string_view CuttedStringView(string &str) {
  auto begin = str.begin();
  auto end = str.end();

  if (begin >= end) {
	return ""sv;
  }

  for (; begin != end; ++begin) {
	if (!IsEscapedChar(*begin)) {
	  break;
	}
  }

  for (; begin != end;) {
	if (!IsEscapedChar(*(end - 1))) {
	  break;
	}
	--end;
  }

  return {&*begin, static_cast<size_t>(end - begin)};
}

Number LoadNumber(istream &input) {
  using namespace literals;

  string parsed_num;

  // Считывает в parsed_num очередной символ из input
  auto read_char = [&parsed_num, &input] {
	parsed_num += static_cast<char>(input.get());
	if (!input) {
	  throw json::ParsingError("Failed to read number from stream"s);
	}
  };

  // Считывает одну или более цифр в parsed_num из input
  auto read_digits = [&input, read_char] {
	if (!isdigit(input.peek())) {
	  throw json::ParsingError("A digit is expected"s);
	}
	while (isdigit(input.peek())) {
	  read_char();
	}
  };

  if (input.peek() == '-') {
	read_char();
  }
  // Парсим целую часть числа
  if (input.peek() == '0') {
	read_char();
	// После 0 в JSON не могут идти другие цифры
  } else {
	read_digits();
  }

  bool is_int = true;
  // Парсим дробную часть числа
  if (input.peek() == '.') {
	read_char();
	read_digits();
	is_int = false;
  }

  // Парсим экспоненциальную часть числа
  if (int ch = input.peek(); ch == 'e' || ch == 'E') {
	read_char();
	if (ch = input.peek(); ch == '+' || ch == '-') {
	  read_char();
	}
	read_digits();
	is_int = false;
  }

  try {
	if (is_int) {
	  // Сначала пробуем преобразовать строку в int
	  try {
		return stoi(parsed_num);
	  } catch (...) {
		// В случае неудачи, например, при переполнении,
		// код ниже попробует преобразовать строку в double
	  }
	}
	return stod(parsed_num);
  } catch (...) {
	throw json::ParsingError("Failed to convert "s + parsed_num + " to number"s);
  }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
string LoadString(istream &input) {
  using namespace literals;

  auto it = istreambuf_iterator<char>(input);
  auto end = istreambuf_iterator<char>();
  string s;
  while (true) {
	if (it == end) {
	  // Поток закончился до того, как встретили закрывающую кавычку?
	  throw json::ParsingError("String parsing error");
	}
	const char ch = *it;
	if (ch == '"') {
	  // Встретили закрывающую кавычку
	  ++it;
	  break;
	} else if (ch == '\\') {
	  // Встретили начало escape-последовательности
	  ++it;
	  if (it == end) {
		// Поток завершился сразу после символа обратной косой черты
		throw json::ParsingError("String parsing error");
	  }
	  const char escaped_char = *(it);
	  // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
	  switch (escaped_char) {
		case 'n': s.push_back('\n');
		  break;
		case 't': s.push_back('\t');
		  break;
		case 'r': s.push_back('\r');
		  break;
		case '"': s.push_back('"');
		  break;
		case '\\': s.push_back('\\');
		  break;
		default:
		  // Встретили неизвестную escape-последовательность
		  throw json::ParsingError("Unrecognized escape sequence \\"s + escaped_char);
	  }
	} else if (ch == '\n' || ch == '\r') {
	  // Строковый литерал внутри- JSON не может прерываться символами \r или \n
	  throw json::ParsingError("Unexpected end of line"s);
	} else {
	  // Просто считываем очередной символ и помещаем его в результирующую строку
	  s.push_back(ch);
	}
	++it;
  }

  return s;
}

nullptr_t LoadNull(istream &input) {
  auto it = istreambuf_iterator<char>(input);
  auto end = istreambuf_iterator<char>();
  string str = string{it, end};

  if (CuttedStringView(str) != "null"sv) {
	throw json::ParsingError(""s + str + " is not equal null and cant be parsed."s);
  }

  return nullptr;
}

bool LoadBool(istream &input) {
  auto it = istreambuf_iterator<char>(input);
  auto end = istreambuf_iterator<char>();

  string src = string{it, end};
  string_view input_str = CuttedStringView(src);

  if (input_str != "true"sv && input_str != "false"sv) {
	throw json::ParsingError(string(input_str.begin(), input_str.end()) + " is cant be parsed as JSON boolean."s);
  }

  return input_str == "true"sv;
}

json::Array LoadArray(istream &input) {
  vector<string> lexems{};
  auto cur_lexem = istreambuf_iterator<char>(input);
  auto end = istreambuf_iterator<char>();
  string cur_lex{};
  json::Array rslt{};
  stack<char> brackets{};
  brackets.push('[');

  while (true) {
	if (cur_lexem == end) throw json::ParsingError("Have not close bracket");

	if (*cur_lexem == '"' || *cur_lexem == '[' || *cur_lexem == '{') {
	  brackets.push(*cur_lexem);
	}

	if (brackets.size() && (*cur_lexem == '"' || *cur_lexem == ']' || *cur_lexem == '}')) {
	  if (open_to_close_bracket.at(brackets.top()) != *cur_lexem) throw json::ParsingError("Close brackets is not matched with open!!!"s);

	  brackets.pop();
	}

	if ((*cur_lexem == ',' && (brackets.size() == 1)) || (brackets.empty() && *cur_lexem == ']')) {
	  if (CuttedStringView(cur_lex).size()) {
		lexems.push_back(cur_lex);
	  }
	  cur_lex.clear();
	} else {
	  cur_lex += *cur_lexem;
	}
	if (*cur_lexem == ']' && brackets.empty()) {
	  ++cur_lexem;
	  break;
	}
	++cur_lexem;
  }

  for (const string &lex: lexems) {
	auto arr_node_stream = istringstream(lex);

	rslt.push_back(LoadNode(arr_node_stream));
  }

  return rslt;
}

json::Dict LoadDict(istream &input) {
  vector<pair<string, string>> lexems{};
  auto cur_lexem = istreambuf_iterator<char>(input);
  auto end = istreambuf_iterator<char>();
  pair<string, string> cur_pair{};
  json::Dict rslt{};
  stack<char> brackets{};
  brackets.push('{');

  bool is_key_entered{true};

  while (true) {
	if (cur_lexem == end) throw json::ParsingError("Have not close bracket");

	if (*cur_lexem == '"' || *cur_lexem == '[' || *cur_lexem == '{') {
	  brackets.push(*cur_lexem);
	}

	if (brackets.size() && (*cur_lexem == '"' || *cur_lexem == ']' || *cur_lexem == '}')) {
	  if (open_to_close_bracket.at(brackets.top()) != *cur_lexem) throw json::ParsingError("Close brackets is not matched with open!!!"s);

	  brackets.pop();
	}

	if (((brackets.size() == 1) && (*cur_lexem == ':' || *cur_lexem == ',')) || ((*cur_lexem == '}') && brackets.empty())) {
	  if (*cur_lexem == ':') {
		is_key_entered = false;
	  } else {
		lexems.push_back(cur_pair);
		cur_pair = {};
		is_key_entered = true;
	  }
	} else {
	  if (is_key_entered) {
		cur_pair.first += *cur_lexem;
	  } else {
		cur_pair.second += *cur_lexem;
	  }
	}
	if (*cur_lexem == '}' && brackets.empty()) {
	  ++cur_lexem;
	  break;
	}
	++cur_lexem;
  }

  for (auto &[key, value]: lexems) {
	auto arr_node_stream = istringstream(value);

	auto cleared = CuttedStringView(key);
	rslt[string{cleared.begin() + 1, cleared.end() - 1}] = LoadNode(arr_node_stream);
  }

  return rslt;
}

json::Node LoadNode(istream &input) {
  char c;
  input >> c;

  if (c == '[') {
	return LoadArray(input);
  } else if (c == '{') {
	return LoadDict(input);
  } else if (c == '"') {
	return LoadString(input);
  } else if (c == 'n') {
	input.putback(c);
	return LoadNull(input);
  } else if (c == 't' || c == 'f') {
	input.putback(c);
	return LoadBool(input);
  } else if (c == '0' || c == '-' || (c >= '1' && c <= '9')) {
	input.putback(c);
	return LoadNumber(input);
  } else {
	throw json::ParsingError("Node lexem is cannot beginning from \""s + c + "\""s);
  }
}

namespace json {

namespace {

}  // namespace
Node::Node() : Node(nullptr) {}
Node::Node(double value) noexcept: value_(value) {}
Node::Node(int value) noexcept: value_(value) {}
Node::Node(bool value) noexcept: value_(value) {}
Node::Node(nullptr_t value) noexcept: value_(value) {}
Node::Node(Array value) noexcept: value_(move(value)) {}
Node::Node(Dict value) noexcept: value_(move(value)) {}
Node::Node(string value) noexcept: value_(move(value)) {}
Node::Node(variant<int, double> value) noexcept {
  variant<int, double> double_ref{2.34};

  if (value.index() == double_ref.index()) {
	value_ = get<double>(value);
  } else {
	value_ = get<int>(value);
  }
}

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
  if (!IsNull()) throw logic_error("Node is not a null"s);

  return get<nullptr_t>(value_);
}
int Node::AsInt() const {
  if (!IsInt()) throw logic_error("Node is not a int"s);

  return get<int>(value_);
}
int Node::AsBool() const {
  if (!IsBool()) throw logic_error("Node is not a bool"s);

  return get<bool>(value_);
}
double Node::AsDouble() const {
  if (!IsInt() && !IsDouble()) throw logic_error("Node is not a double or int"s);

  if (IsInt()) {
	return static_cast<double>(get<int>(value_));
  }

  return get<double>(value_);
}
double Node::AsPureDouble() const {
  if (!IsDouble()) throw logic_error("Node is not a double"s);

  return get<double>(value_);
}
const string &Node::AsString() const {
  if (!IsString()) throw logic_error("Node is not a string"s);

  return get<string>(value_);
}
const Array &Node::AsArray() const {
  if (!IsArray()) throw logic_error("Node is not a Array"s);

  return get<Array>(value_);
}
const Dict &Node::AsMap() const {
  if (!IsMap()) throw logic_error("Node is not a object"s);

  return get<Dict>(value_);
}
bool Node::operator==(const Node &rhs) const {
  if (value_.index() != rhs.value_.index()) return false;

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

  return get<string>(GetValue()) == get<string>(rhs.GetValue());
}
bool Node::operator!=(const Node &rhs) const {
  return !this->operator==(rhs);
}

Document::Document(Node root)
	: root_(move(root)) {
}

const Node &Document::GetRoot() const {
  return root_;
}

Document Load(istream &input) {
  return Document{LoadNode(input)};
}

void Print(const Document &doc, ostream &output) {
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
  for (auto iter = obj.begin(); iter != obj.end(); ++iter) {
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
  string stringified_double = to_string(double_val);

  for (size_t i = 0; i != stringified_double.size(); ++i) {
	if (stringified_double[stringified_double.size() - i - 1] == '.') {
	  return {stringified_double.begin(), stringified_double.end() - i + 1};
	}

	if (stringified_double[stringified_double.size() - i - 1] != '0') {
	  return {stringified_double.begin(), stringified_double.end() - i};
	}
  }

  return stringified_double;
}
string NodeStringifier::operator()(const string &str_val) {
  string rslt{};

  rslt.reserve(str_val.size() + 4);

  rslt.push_back('\"');

  for (auto iter = str_val.begin(); iter < str_val.end(); ++iter) {
	switch (*iter) {
	  case '\\': rslt.push_back('\\');
		rslt.push_back('\\');
		break;
	  case '\r': rslt.push_back('\\');
		rslt.push_back('r');
		break;
	  case '\n': rslt.push_back('\\');
		rslt.push_back('n');
		break;
	  case '\t': rslt.push_back('\t');
		break;
	  case '"': rslt.push_back('\\');
		rslt.push_back(*iter);
		break;
	  default: rslt.push_back(*iter);
		break;
	}
  }

  rslt.push_back('\"');

  return rslt;
}

bool operator==(const json::Document &lhs, const json::Document &rhs) {
  return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(const json::Document &lhs, const json::Document &rhs) {
  return !(lhs == rhs);
}
}  // namespace json