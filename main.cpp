#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;

int main()
{
  TransportCatalogue catalogue{};

  auto db_command_query = user_input_db_command();

  for (const auto &command : db_command_query)
  {
    catalogue.apply_db_command(command);
  }

  auto output_command_query = user_input_output_command();

  for (const auto &command : output_command_query)
  {
    catalogue.apply_output_command(cout, command);
  }

  return 0;
}
