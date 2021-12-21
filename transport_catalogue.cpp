#include "transport_catalogue.h"

using namespace std;
using namespace literals::string_literals;
using namespace literals::string_view_literals;

void TransportCatalogue::apply_db_command(const string &command)
{

}

void TransportCatalogue::apply_output_command(const string &command)
{

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
    case OutputCommands::PrintStop:
      return GetMetadataQueryForPrintStop(command);
    default:
      throw invalid_command_code();
  }
}

vector<string_view> TransportCatalogue::GetMetadataQueryForAddBus(const string_view &command)
{
  return vector<string_view>();
}

vector<string_view> TransportCatalogue::GetMetadataQueryForAddStop(const string_view &command)
{
  return vector<string_view>();
}

vector<string_view> TransportCatalogue::GetMetadataQueryForPrintBus(const string_view &command)
{
  return vector<string_view>();
}

vector<string_view> TransportCatalogue::GetMetadataQueryForPrintStop(const string_view &command)
{
  return vector<string_view>();
}

bool TransportCatalogue::IsClosedRoute(const Route *route)
{
  return *route->begin() == route->at(route->size() - 1);
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
  else if (command_key_and_meta.first == "Stop"sv)
  {
    command_key = OutputCommands::PrintStop;
  }
  else
  {
    throw invalid_command_metadata();
  }

  return {command_key, command_key_and_meta.second};
}

BusMeta TransportCatalogue::MakeBusMetaFrom(string_view meta_query)
{
  return BusMeta();
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
  return stop{stop_meta.first, stop_meta.second};
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

void TransportCatalogue::add_route(const Route &&route)
{
  routes_.push_back(route);
}

void TransportCatalogue::connect_bus_and_stop(size_t bus_index, size_t stop_index)
{
  buses_to_stops_[bus_index].push_back(stop_index);
  stops_to_buses_[stop_index].push_back(bus_index);
}

void TransportCatalogue::connect_stop_and_route(size_t stop_index, size_t route_index)
{
  routes_.at(route_index).push_back(stop_index);
  stops_to_routes_.at(stop_index).push_back(route_index);
}

void TransportCatalogue::remove_stop(size_t stop_index)
{
  for (size_t bus_index : stops_to_buses_.at(stop_index))
  {
    auto target_vector = buses_to_stops_.at(bus_index);

    for (auto iter = target_vector.begin(); iter != target_vector.end();)
    {
      if (*iter == stop_index)
      {
        iter = target_vector.erase(iter);
      }
      else
      {
        ++iter;
      }
    }
  }

  names_to_stops_.erase(stops_[stop_index].name);
  stops_to_buses_.erase(stop_index);
  stops_to_routes_.erase(stop_index);
  stops_.erase(stops_.begin() + static_cast<int>(stop_index));
}

void TransportCatalogue::remove_bus(size_t bus_index)
{

}

void TransportCatalogue::remove_route(size_t route_index)
{}

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
