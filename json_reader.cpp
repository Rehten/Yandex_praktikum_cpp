#include <vector>
#include <sstream>
#include <numeric>

#include "json.h"
#include "json_reader.h"

using std::istream;
using std::ios;
using std::vector;
using std::istringstream;
using std::string;
using std::reduce;
using std::plus;

json::Document
JSONReader::get_json_as_document() const
{
  string flatten_json = reduce(
    json_raws_.begin(),
    json_raws_.end(),
    string{},
    plus()
  );
  istringstream is = istringstream(flatten_json);

  return json::Load(is);
}

istream&
operator >>(istream& is, JSONReader& json_reader)
{
  vector<string> json_raw{};
  string cur_line{};
  bool is_json_parsed(false);

  while (!is_json_parsed)
  {
    cur_line.clear();
    getline(is, cur_line, '\n');

    json_raw.push_back(cur_line);

    try
    {
      string flatten_json = reduce(
        json_raw.begin(),
        json_raw.end(),
        string{},
        plus()
      );
      istringstream is_test_stream = istringstream(flatten_json);

      json::Load(is_test_stream);
      is_json_parsed = true;
    }
    catch (...)
    {}
  }

  json_reader.json_raws_ = json_raw;

  return is;
}

std::ostream&
operator <<(std::ostream& os, JSONReader& json_reader)
{
  json::Print(json_reader.get_json_as_document(), os);

  return os;
}
JSONReader::operator bool()
{
  return !json_raws_.empty();
}
