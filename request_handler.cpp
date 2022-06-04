#define _USE_MATH_DEFINES

#include <iostream>
#include <utility>
#include <numeric>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <sstream>

#include "request_handler.h"

using namespace std;
using namespace literals::string_literals;
using namespace literals::string_view_literals;

vector<RequestHandler::DBCommandQuery>
RawRequestHandler::get_db_commands_from(istream& is)
{
  auto get_db_commands = [&is](size_t count) -> vector<string>
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
  };

  auto user_input_db_command = [&get_db_commands](istream& is) -> vector<string>
  {
    size_t commands_count{};

    is >> commands_count;

    is.clear();
    is.ignore();

    return get_db_commands(commands_count);
  };

  auto user_input_commands = user_input_db_command(is);
  vector<DBCommandQuery> db_commands{};
  db_commands.reserve(user_input_commands.size());

  for (size_t i = 0; i != user_input_commands.size(); ++i)
  {
    const auto& str = user_input_commands[i];
    auto db_command = QueryParser::GetDBCommandCodeAndQuery(str);
    string command_meta = string(db_command.second.begin(), db_command.second.end());

    db_commands.push_back({db_command.first, command_meta});
  }

  return db_commands;
}

vector<RequestHandler::OutputCommandQuery>
RawRequestHandler::get_output_commands_from(istream& is)
{
  auto get_output_commands = [&is](size_t count) -> vector<string>
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
  };

  auto user_input_output_command = [&get_output_commands](istream& is) -> vector<string>
  {
    size_t commands_count{};

    is >> commands_count;

    is.clear();
    is.ignore();

    return get_output_commands(commands_count);
  };

  auto user_commands = user_input_output_command(is);
  vector<OutputCommandQuery> output_commands{};
  output_commands.reserve(user_commands.size());

  for (size_t i = 0; i != user_commands.size(); ++i)
  {
    const auto& str = user_commands[i];
    auto output_command = QueryParser::GetOutputCommandCodeAndQuery(str);
    string command_meta = string(output_command.second.begin(), output_command.second.end());

    output_commands.push_back({output_command.first, command_meta}
    );
  }

  return output_commands;
}

#if __HAS_JSON_SUPPORT__
vector<JSONRequestHandler::DBCommandQuery>
JSONRequestHandler::get_db_commands_from(istream& is)
{
  if (!json_reader_) is >> json_reader_;

  json::Document document = json_reader_.get_json_as_document();
  json::Array requests = document.GetRoot().AsMap().at("base_requests"s).AsArray();
  vector<string> raw_commands{};

  for (auto& request: requests)
  {
    raw_commands.push_back(dcq_from_json(request.AsMap()));
  }

  stringstream string_stream(
    to_string(raw_commands.size())
      + "\n"s
      + reduce(
        raw_commands.begin(),
        raw_commands.end(),
        ""s,
        plus()
      )
  );

  return raw_request_handler_.get_db_commands_from(string_stream);
}

vector<JSONRequestHandler::OutputCommandQuery>
JSONRequestHandler::get_output_commands_from(istream& is)
{
  if (!json_reader_) is >> json_reader_;

  json::Document document = json_reader_.get_json_as_document();
  json::Array requests = document.GetRoot().AsMap().at("stats_requests"s).AsArray();
  vector<string> raw_commands{};

  for (auto& request: requests)
  {
    raw_commands.push_back(ocq_from_json(request.AsMap()));
  }

  stringstream string_stream(
    to_string(raw_commands.size())
      + "\n"s
      + reduce(
        raw_commands.begin(),
        raw_commands.end(),
        ""s,
        plus()
      )
  );

  return raw_request_handler_.get_output_commands_from(string_stream);
}

string
JSONRequestHandler::dcq_from_json(const json::Dict&) noexcept
{
  return ""s;
}

string
JSONRequestHandler::ocq_from_json(const json::Dict&) noexcept
{
  return ""s;
}
#endif

void
RawResponseSeller::send_bus(
  TransportCatalogue* tc_ptr,
  ostream& os,
  std::vector<std::string_view> command_lexems
)
{
  string bus_id = string(command_lexems[0].begin(), command_lexems[0].end());

  os << "Bus "s << bus_id << ": "s;

  if (!tc_ptr->ids_to_buses_.count(bus_id))
  {
    os << "not found"s << endl;
  }
  else
  {
    auto selected_bus_stops = tc_ptr->buses_to_stops_.at(tc_ptr->ids_to_buses_.at(bus_id));

    if (selected_bus_stops.size() < 2)
    {
      throw invalid_command_metadata();
    }

    size_t routes_count = selected_bus_stops.size();
    size_t unique_routes_count =
      set(selected_bus_stops.begin(), selected_bus_stops.end()).size();
    bool is_practical_length_can_be_calculated(true);
    double theoretical_routes_length{};
    double practical_routes_length{};

    for (size_t i = 1; i != selected_bus_stops.size(); ++i)
    {
      stop prev = tc_ptr->stops_[selected_bus_stops[i - 1]];
      stop cur = tc_ptr->stops_[selected_bus_stops[i]];

      theoretical_routes_length +=
        ComputeDistance(*prev.coordinates, *cur.coordinates);

      if (
        is_practical_length_can_be_calculated &&
          tc_ptr->stops_to_stop_distances_.count(prev.name)
          &&
            tc_ptr->stops_to_stop_distances_.at(prev.name).count(cur.name))
      {
        practical_routes_length += static_cast<double>(
          tc_ptr->stops_to_stop_distances_
            .at(prev.name)
            .at(cur.name)
        );
      }
      else
      {
        is_practical_length_can_be_calculated = false;
      }
    }

    os << routes_count << " stops on route, "s
       << unique_routes_count << " unique stops, "s
       << (is_practical_length_can_be_calculated
           ? practical_routes_length : theoretical_routes_length)
       << " route length, "s
       << static_cast<double>(
         static_cast<double>(practical_routes_length)
           / (round(theoretical_routes_length * 100) / 100))
       << " curvature"s << endl;
  }
}

void
RawResponseSeller::send_stop(
  TransportCatalogue* tc_ptr,
  ostream& os,
  std::vector<std::string_view> command_lexems
)
{
  string query_stop_name = string(command_lexems[0].begin(), command_lexems[0].end());

  if (!tc_ptr->names_to_stops_.count(query_stop_name))
  {
    os << "Stop " << query_stop_name << ": not found" << endl;
    return;
  }

  size_t stop_index = tc_ptr->names_to_stops_.at(query_stop_name);

  if (!tc_ptr->stops_to_buses_.count(stop_index)
    || tc_ptr->stops_to_buses_.at(stop_index).empty())
  {
    os << "Stop " << query_stop_name << ": no buses" << endl;
    return;
  }

  auto& buses_indexes =
    tc_ptr->stops_to_buses_.at(tc_ptr->names_to_stops_.at(query_stop_name));
  vector<string> buses_ids{};

  buses_ids.reserve(buses_indexes.size());

  os << "Stop " << query_stop_name << ": buses ";
  for (size_t bus_index: buses_indexes)
  {
    buses_ids.push_back(tc_ptr->buses_[bus_index].id);
  }

  sort(buses_ids.begin(), buses_ids.end(), less());

  for (const string& id: buses_ids)
  {
    os << id << " ";
  }
  os << endl;
}

#if __HAS_JSON_SUPPORT__
void
JSONResponseSeller::send_bus(TransportCatalogue*, std::ostream&, std::vector<std::string_view>)
{}

void
JSONResponseSeller::send_stop(TransportCatalogue*, std::ostream&, std::vector<std::string_view>)
{}
#endif
