#include <iostream>
#include "transport_catalogue.h"

using namespace std;
using namespace literals::string_literals;
using namespace literals::string_view_literals;

void TransportCatalogue::apply_db_command(const string &command)
{
  pair<DBCommands, string_view> db_parsed_command = GetDBCommandCodeAndQuery(command);
  vector<string_view> query = GetMetadataQueryByCode(db_parsed_command.first, db_parsed_command.second);

  switch (db_parsed_command.first)
  {
    case DBCommands::AddBus:
      add_bus(BuildBusFrom(MakeBusMetaFrom(db_parsed_command.second)));
      break;
    case DBCommands::AddStop:
      add_stop(BuildStopFrom(MakeStopMetaFrom(db_parsed_command.second)));
      break;
    default:
      throw invalid_command_code();
  }
}

void TransportCatalogue::apply_output_command(ostream &output_stream, const string &command)
{
  pair<OutputCommands, string_view> db_command_meta = GetOutputCommandCodeAndQuery(command);
  vector<string_view> query = GetMetadataQueryByCode(db_command_meta.first, db_command_meta.second);

  switch (db_command_meta.first)
  {
    case OutputCommands::PrintBus:
      {
        size_t bus_id = stoi(string(query[0].begin(),  query[0].end()));
        size_t printed_bus_id = ids_to_buses_.at(bus_id);

        output_stream << "Bus "s << buses_[printed_bus_id].id << ": not found"s;
      }
      break;
    default:
      throw invalid_command_code();
  }
}

vector<string_view> TransportCatalogue::GetMetadataQueryByCode(size_t code, const string_view &command)
{
  switch (code)
  {
    case DBCommands::AddBus:
      return GetMetadataQueryForAddBus(command);
    case DBCommands::AddStop:
      return GetMetadataQueryForAddStop(command);
    case OutputCommands::PrintBus:
      return GetMetadataQueryForPrintBus(command);
    default:
      throw invalid_command_code();
  }
}

vector<string_view> TransportCatalogue::GetMetadataQueryForAddBus(const string_view &command)
{
  vector<string_view> bus_metadata_query{};
  bus_metadata_query.reserve(3);
  string_view::iterator lexem_begin = command.begin();
  bool is_bus_id_getted(false);
  bool is_route_need_reversed(false);

  for (auto iter = command.begin(); iter != command.end(); ++iter)
  {
    if (!is_bus_id_getted)
    {
      if (*iter == ':')
      {
        is_bus_id_getted = true;
        bus_metadata_query.emplace_back(lexem_begin, iter);
        lexem_begin = iter + 2 < command.end() ? iter + 2 : throw invalid_command_metadata();
        ++iter;
      }
    }
    else
    {
      if (*iter == '-')
      {
        is_route_need_reversed = true;
      }

      if (command.end() - iter == 1)
      {
        bus_metadata_query.emplace_back(lexem_begin, command.end());
      }

      if (*iter == '-' || *iter == '>')
      {
        bus_metadata_query.emplace_back(lexem_begin, iter - 1);
        lexem_begin = iter + 2 < command.end() ? iter + 2 : throw invalid_command_metadata();
      }
    }
  }

  if (is_route_need_reversed)
  {
    bus_metadata_query.reserve(bus_metadata_query.size() * 2 - 1);
    for (auto iter = bus_metadata_query.rbegin() + 1; iter != bus_metadata_query.rend() - 1; ++iter)
    {
      bus_metadata_query.push_back(*iter);
    }
  }

  return bus_metadata_query;
}

vector<string_view> TransportCatalogue::GetMetadataQueryForAddStop(const string_view &command)
{
  vector<string_view> stop_metadata_query{};
  stop_metadata_query.reserve(3);
  string_view::iterator lexem_begin = command.begin();
  bool is_stop_name_getted(false);
  bool is_latitude_getted(false);

  for (auto iter = command.begin(); iter != command.end(); ++iter)
  {
    if (!is_stop_name_getted)
    {
      if (*iter == ':')
      {
        is_stop_name_getted = true;
        stop_metadata_query.emplace_back(lexem_begin, iter);
        lexem_begin = iter + 2 < command.end() ? iter + 2 : throw invalid_command_metadata();
        ++iter;
      }
    }
    else if (!is_latitude_getted)
    {
      if (*iter == ',')
      {
        is_latitude_getted = true;
        stop_metadata_query.emplace_back(lexem_begin, iter);
        lexem_begin = iter + 2 < command.end() ? iter + 2 : throw invalid_command_metadata();
        ++iter;
      }
    }
    else
    {
      if (command.end() - iter == 1)
      {
        stop_metadata_query.emplace_back(lexem_begin, command.end());
      }
    }
  }

  return stop_metadata_query;
}

vector<string_view> TransportCatalogue::GetMetadataQueryForPrintBus(const string_view &command)
{
  return {{command.begin(),  command.end()}};
}

pair<DBCommands, string_view> TransportCatalogue::GetDBCommandCodeAndQuery(const string &from)
{
  if (from.empty())
  {
    throw empty_command_metadata();
  }

  pair<string_view, string_view> command_key_and_meta = DivideCommandByCodeAndValue(from);
  DBCommands command_key{};

  if (command_key_and_meta.first == "Bus"sv)
  {
    command_key = DBCommands::AddBus;
  }
  else if (command_key_and_meta.first == "Stop"sv)
  {
    command_key = DBCommands::AddStop;
  }
  else
  {
    throw invalid_command_metadata();
  }

  return {command_key, command_key_and_meta.second};
}

pair<OutputCommands, string_view> TransportCatalogue::GetOutputCommandCodeAndQuery(const string &from)
{
  if (from.empty())
  {
    throw empty_command_metadata();
  }

  pair<string_view, string_view> command_key_and_meta = DivideCommandByCodeAndValue(from);
  OutputCommands command_key{};

  if (command_key_and_meta.first == "Bus"sv)
  {
    command_key = OutputCommands::PrintBus;
  }
  else
  {
    throw invalid_command_metadata();
  }

  return {command_key, command_key_and_meta.second};
}

BusMeta TransportCatalogue::MakeBusMetaFrom(string_view meta_query)
{
  return {static_cast<size_t>(stoi(string(meta_query.begin(),  meta_query.end()))), {}};
}

StopMeta TransportCatalogue::MakeStopMetaFrom(string_view meta_query)
{
  return StopMeta();
}

bus TransportCatalogue::BuildBusFrom(BusMeta bus_meta)
{
  return {bus_meta.first};
}

stop TransportCatalogue::BuildStopFrom(StopMeta stop_meta)
{
  return stop{0, stop_meta.first, stop_meta.second};
}

pair<string_view, string_view> TransportCatalogue::DivideCommandByCodeAndValue(const string &src)
{
  string_view command_code{};
  string_view command_meta{};

  for (auto iter = src.begin(); iter != src.end(); ++iter)
  {
    if (*iter == ' ')
    {
      command_code = {src.begin(), iter};
      command_meta = {iter + 1, src.end()};
      break;
    }
  }

  return {command_code, command_meta};
}

void TransportCatalogue::add_stop(const stop &&stop)
{
  stops_.push_back(stop);
}

void TransportCatalogue::add_bus(const bus &&bus)
{
  buses_.push_back(bus);
  ids_to_buses_[bus.id] = buses_.size() - 1;
}

void TransportCatalogue::add_route(const route &&route)
{
}

void TransportCatalogue::connect_bus_and_stop(size_t bus_index, size_t stop_index)
{

}

void TransportCatalogue::connect_stop_and_route(size_t stop_index, size_t route_index)
{

}

void TransportCatalogue::clear()
{
  routes_.clear();
  stops_.clear();
  buses_.clear();
  names_to_stops_.clear();
  stops_to_buses_.clear();
  buses_to_stops_.clear();
  stops_to_routes_.clear();
  ids_to_buses_.clear();
}
