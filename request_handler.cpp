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

  vector<DBCommandQuery> db_commands{};
  db_commands.reserve(raw_commands.size());

  for (size_t i = 0; i != raw_commands.size(); ++i)
  {
    const auto& str = raw_commands[i];
    auto db_command = QueryParser::GetDBCommandCodeAndQuery(str);
    string command_meta = string(db_command.second.begin(), db_command.second.end());

    db_commands.push_back({db_command.first, command_meta});
  }

  return db_commands;
}

vector<JSONRequestHandler::OutputCommandQuery>
JSONRequestHandler::get_output_commands_from(istream& is)
{
  if (!json_reader_) is >> json_reader_;

  json::Document document = json_reader_.get_json_as_document();
  json::Array requests = document.GetRoot().AsMap().at("stat_requests"s).AsArray();
  vector<string> raw_commands{};

  for (auto& request: requests)
  {
    raw_commands.push_back(ocq_from_json(request.AsMap()));
  }

  vector<OutputCommandQuery> output_commands{};
  output_commands.reserve(raw_commands.size());

  for (size_t i = 0; i != raw_commands.size(); ++i)
  {
    const auto& str = raw_commands[i];
    auto db_command = QueryParser::GetOutputCommandCodeAndQuery(str);
    string command_meta = string(db_command.second.begin(), db_command.second.end());

    output_commands.push_back({db_command.first, command_meta});
  }

  return output_commands;
}

string
JSONRequestHandler::dcq_from_json(const json::Dict& command_json) noexcept
{
  string raw_command{};

  if (command_json.at("type"s) == "Bus"s)
  {
    raw_command = get_raw_db_bus_command_from(command_json);
  }
  else if (command_json.at("type"s) == "Stop"s)
  {
    raw_command = get_raw_db_stop_command_from(command_json);
  }

  return raw_command;
}

string
JSONRequestHandler::ocq_from_json(const json::Dict& command_json) noexcept
{
  string raw_command{};

  if (command_json.at("type"s) == "Bus"s)
  {
    raw_command = get_raw_output_bus_command_from(command_json);
  }
  else if (command_json.at("type"s) == "Stop"s)
  {
    raw_command = get_raw_output_stop_command_from(command_json);
  }

  RequestsIDsList.push_back(static_cast<size_t>(command_json.at("id").AsInt()));

  return raw_command;
}
std::string
JSONRequestHandler::get_raw_db_bus_command_from(const json::Dict& bus_command_json) noexcept
{
  string raw_command = {};
  raw_command.reserve(100);
  json::Array stops_json = bus_command_json.at("stops"s).AsArray();
  char divider = '>';

  if (stops_json.size() && *stops_json.begin() != *stops_json.rbegin())
  {
    divider = '-';
  }

  raw_command += "Bus "s + bus_command_json.at("name"s).AsString() + ": "s;

  for (size_t i = 0; i != stops_json.size(); ++i)
  {
    if (i)
    {
      raw_command += " "s + divider + " "s;
    }

    raw_command += stops_json[i].AsString();
  }

  return raw_command;
}
std::string
JSONRequestHandler::get_raw_db_stop_command_from(const json::Dict& stop_command_json) noexcept
{
  string raw_command = {};
  raw_command.reserve(100);
  json::Dict distantions = stop_command_json.at("road_distances"s).AsMap();

  raw_command += "Stop "s
    + stop_command_json.at("name"s).AsString()
    + ": "
    + to_string(stop_command_json.at("latitude"s).AsDouble())
    + ", "
    + to_string(stop_command_json.at("longitude"s).AsDouble());

  for (const auto& [stop_name, distantion] : distantions)
  {
    raw_command += ", "s + to_string(distantion.AsInt()) + "m to "s + stop_name;
  }

  return raw_command;
}
std::string
JSONRequestHandler::get_raw_output_bus_command_from(const json::Dict& bus_command_json) noexcept
{
  return "Bus "s + bus_command_json.at("name"s).AsString();
}
std::string
JSONRequestHandler::get_raw_output_stop_command_from(const json::Dict& stop_command_json) noexcept
{
  return "Stop "s + stop_command_json.at("name"s).AsString();
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
JSONResponseSeller::send_bus(
  TransportCatalogue* tc_ptr,
  ostream& os,
  std::vector<std::string_view> command_lexems
)
{
  ApplyRenderStart(os);

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

    ApplyRenderBetween(os);
    ApplyRenderEnd(os);
  }
}

void
JSONResponseSeller::send_stop(
  TransportCatalogue* tc_ptr,
  ostream& os,
  std::vector<std::string_view> command_lexems
)
{
  ApplyRenderStart(os);
  os << "  { "s << "\"request_id\": "s << to_string(RequestsIDsList[RenderedRequestIndex]) << ", "s;

  string query_stop_name = string(command_lexems[0].begin(), command_lexems[0].end());

  if (!tc_ptr->names_to_stops_.count(query_stop_name))
  {
    os << "\"error_messsage\": \"not found\"" << endl;
  }
  else
  {
    size_t stop_index = tc_ptr->names_to_stops_.at(query_stop_name);

    if (!tc_ptr->stops_to_buses_.count(stop_index)
      || tc_ptr->stops_to_buses_.at(stop_index).empty())
    {
      os << "\"buses\": [] " << endl;
    }
    else
    {
      auto& buses_indexes = tc_ptr->stops_to_buses_.at(tc_ptr->names_to_stops_.at(query_stop_name));
      vector<string> buses_ids{};

      buses_ids.reserve(buses_indexes.size());

      for (size_t bus_index: buses_indexes)
      {
        buses_ids.push_back(tc_ptr->buses_[bus_index].id);
      }

      sort(buses_ids.begin(), buses_ids.end(), less());

      os << "\"buses\": [ "s;
      for (size_t i = 0; i != buses_ids.size(); ++i)
      {
        const string& id = buses_ids[i];

        if (i) os << ", "s;

        os << "\""s << id << "\""s;
      }
      os << " ] "s;
    }
  }

  os << "}"s;
  ApplyRenderBetween(os);
  ApplyRenderEnd(os);
}
void
JSONResponseSeller::ApplyRenderStart(ostream& os)
{
  if (!RenderedRequestIndex) os << "["s << endl;
}
void
JSONResponseSeller::ApplyRenderBetween(ostream& os)
{
  if (RenderedRequestIndex != RequestsIDsList.size() - 1) os << ","s;

  os << endl;
}
void
JSONResponseSeller::ApplyRenderEnd(ostream& os)
{
  if (RenderedRequestIndex == RequestsIDsList.size() - 1)
  {
    os << "]"s;
    RenderedRequestIndex = 0;
    RequestsIDsList.clear();
  }
  else
  {
    ++RenderedRequestIndex;
  }
}
#endif
