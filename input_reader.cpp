#include <iostream>

#include "input_reader.h"

using std::cin;
using std::istream;
using std::vector;
using std::string;
using std::istream;
using std::getline;

vector<string> get_db_commands(istream &is, size_t count)
{
  vector<string> rslt{};
  string command{};

  rslt.reserve(count);

  while (count--)
  {
    getline(is, command, '\n');
    rslt.push_back(command);
    command.clear();
  }

  return rslt;
}

vector<string> user_input_db_command()
{
  size_t commands_count {};

  cin >> commands_count;

  cin.clear();
  cin.ignore();

  return get_db_commands(cin, commands_count);
}
