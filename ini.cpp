#include "ini.h"

using namespace std;

ini::Section &ini::Document::AddSection(string name)
{
  return sections_[name];
}

const ini::Section &ini::Document::GetSection(const string &name) const
{
  return sections_.at(name);
}

size_t ini::Document::GetSectionCount() const
{
  return sections_.size();
}

ini::Document ini::Load(istream &input)
{
  Document document{};
  string line{};
  string last_added{};

  string added_section_name{};

  string added_section_parameter_name{};
  string added_section_parameter_value{};

  while (getline(input, line))
  {
    pair<string, string> lexems{};
    string current_lexem{};
    size_t lexem_start{};

    bool is_lexem_parsing_started(false);
    bool is_added_section(false);

    for (size_t i = 0; i < line.size(); ++i)
    {
      char c = line[i];

      if (is_lexem_parsing_started)
      {
        LexemParsing:
        if (is_added_section)
        {
          if (c == '[')
          {
            lexem_start = i;
          }
          if (c == ']')
          {
            lexems.first = string{&line[lexem_start + 1], &line[i]};
            break;
          }
        }
        else
        {
          if (c == '=')
          {
            lexem_start = i;
            lexems.first = string{&line[0], &line[lexem_start]};
            lexems.second = string{&line[lexem_start + 1], &line[line.size()]};
            break;
          }
        }
      }
      else
      {
        if (c != ' ')
        {
          if (c == '[')
          {
            is_added_section = true;
          }
          is_lexem_parsing_started = true;
          goto LexemParsing;
        }
      }
    }

    if (lexems.second.empty())
    {
      document.sections_[lexems.first];
      last_added = lexems.first;
    }
    else
    {
      document.sections_[last_added][lexems.first] = lexems.second;
    }
  }

  return document;
}

