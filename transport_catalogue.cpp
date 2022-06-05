#include <iostream>
#include <utility>
#include <numeric>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <iomanip>

#include "transport_catalogue.h"

#define DB_COMMAND_QUERY_MIN_LEXEMS_COUNT 3

using namespace std;
using namespace geo;
using namespace literals::string_literals;
using namespace literals::string_view_literals;

TransportCatalogue::TransportCatalogue(
  unique_ptr<RequestHandler>&& request_handler,
  unique_ptr<ResponseSeller>&& response_seller_ptr
)
  : request_handler_ptr_(move(request_handler)), response_seller_ptr_(move(response_seller_ptr))
{
}

void
TransportCatalogue::listen_db_commands_from(std::istream& is)
{
  const auto db_commands_query = request_handler_ptr_->get_db_commands_from(is);

  for (const auto& command: db_commands_query)
  {
    apply_db_command(command);
  }
}

void
TransportCatalogue::listen_output_commands_from(std::istream& is, std::ostream& os)
{
  auto output_command_query = request_handler_ptr_->get_output_commands_from(is);

  os << setprecision(6);

  for (const auto& command: output_command_query)
  {
    apply_output_command(cout, command);
  }
}

void
TransportCatalogue::apply_db_command(const pair<DBCommands, string>& db_parsed_command)
{
  switch (db_parsed_command.first)
  {
    case DBCommands::AddBus:
    {
      auto busmeta = MakeBusMetaFrom(db_parsed_command.second);
      vector<string> stops{};
      route added_route{};

      add_bus(BuildBusFrom(busmeta));

      stops.resize(busmeta.second.size());
      added_route.stops.resize(stops.size());

      transform(busmeta.second.begin(),
                busmeta.second.end(),
                stops.begin(),
                [](const string_view& stop_sv) -> string
                {
                  return {stop_sv.begin(), stop_sv.end()};
                }
      );

      for (const string& stopname: stops)
      {
        if (!names_to_stops_.count(stopname))
        {
          add_stop({++last_stop_id_, {stopname.begin(), stopname.end()},
                    nullopt}
          );
        }
        connect_bus_and_stop(ids_to_buses_.at(busmeta.first),
                             names_to_stops_.at(stopname));
      }

      transform(stops.begin(),
                stops.end(),
                added_route.stops.begin(),
                [this](const string& stopname) -> size_t
                {
                  return names_to_stops_.at(stopname);
                }
      );

      add_route(move(added_route));

      for (size_t stop_index: set(routes_[routes_.size() - 1].stops.begin(),
                                  routes_[routes_.size() - 1].stops.end()))
      {
        connect_stop_and_route(stop_index, routes_.size() - 1);
      }
    }
      break;
    case DBCommands::AddStop:
    {
      auto stopmeta = MakeStopMetaFrom(db_parsed_command.second);
      string stopname = stopmeta.name;

      if (!names_to_stops_.count(stopname))
      {
        add_stop(build_stop_from(stopmeta));
      }
      else
      {
        // Проставляет координаты, если остановка была неявно создана(например при создании автобуса) до вызова явной команды на создание
        if (stops_[names_to_stops_.at(stopname)].coordinates.has_value())
        {
          throw stop_already_has_coordinates();
        }
        stops_[names_to_stops_.at(stopname)].coordinates = stopmeta.coordinates;
      }

      if (stopmeta.dependencies.size() || stopmeta.dependencies.size() % 2)
      {
        write_stop_dependency(stopmeta, stopname);
      }
    }
      break;
    default:throw invalid_command_code();
  }
}

void
TransportCatalogue::apply_output_command(
  ostream& output_stream,
  const pair<OutputCommands, string>& output_command_meta
)
{
  vector<string_view> query =
    QueryParser::GetMetadataQueryByCode(static_cast<size_t>(output_command_meta.first), output_command_meta.second);

  switch (output_command_meta.first)
  {
    case OutputCommands::PrintBus:
      response_seller_ptr_->send_bus(this, output_stream, query);
      break;
    case OutputCommands::PrintStop:
      response_seller_ptr_->send_stop(this, output_stream, query);
      break;
    default: throw invalid_command_code();
  }
}

vector<string_view>
QueryParser::GetMetadataQueryByCode(
  size_t code,
  const string_view& command
)
{
  switch (code)
  {
    case static_cast<size_t>(DBCommands::AddBus): return GetMetadataQueryForAddBus(command);
    case static_cast<size_t>(DBCommands::AddStop): return GetMetadataQueryForAddStop(command);
    case static_cast<size_t>(OutputCommands::PrintBus): return GetMetadataQueryForPrintBus(command);
    case static_cast<size_t>(OutputCommands::PrintStop): return GetMetadataQueryForPrintStop(command);
    default: throw invalid_command_code();
  }
}

BusMeta
TransportCatalogue::MakeBusMetaFrom(string_view meta_query)
{
  vector<string_view> splitted_meta_query =
    QueryParser::GetMetadataQueryByCode(static_cast<size_t>(DBCommands::AddBus), meta_query);

  return {
    string(
      splitted_meta_query[0].begin(),
      splitted_meta_query[0].end()
    ),
    {splitted_meta_query.begin() + 1, splitted_meta_query.end()}
  };
}

StopMeta
TransportCatalogue::MakeStopMetaFrom(string_view meta_query)
{
  vector<string_view> splitted_meta_query =
    QueryParser::GetMetadataQueryByCode(static_cast<size_t>(DBCommands::AddStop), meta_query);

  if (splitted_meta_query.size() < DB_COMMAND_QUERY_MIN_LEXEMS_COUNT)
  {
    throw invalid_command();
  }

  auto dependencies =
    splitted_meta_query.size() == DB_COMMAND_QUERY_MIN_LEXEMS_COUNT
    ? vector<string_view>{} : vector<string_view>{
      splitted_meta_query.begin() + 3,
      splitted_meta_query.end()};

  return {
    {splitted_meta_query[0].begin(), splitted_meta_query[0].end()},
    Coordinates{
      stod(string{splitted_meta_query[1].begin(),
                  splitted_meta_query[1].end()}
      ),
      stod(string{splitted_meta_query[2].begin(), splitted_meta_query[2].end()})
    },
    dependencies
  };
}

bus
TransportCatalogue::BuildBusFrom(BusMeta bus_meta)
{
  return {bus_meta.first};
}

stop
TransportCatalogue::build_stop_from(StopMeta stop_meta)
{
  return stop{++last_stop_id_, stop_meta.name, stop_meta.coordinates};
}

void
TransportCatalogue::add_stop(const stop&& stop)
{
  stops_.push_back(stop);
  names_to_stops_[stops_[stops_.size() - 1].name] = stops_.size() - 1;
}

void
TransportCatalogue::add_bus(const bus&& bus)
{
  buses_.push_back(bus);
  ids_to_buses_[bus.id] = buses_.size() - 1;
}

void
TransportCatalogue::add_route(const route&& route)
{
  routes_.push_back(route);
}

void
TransportCatalogue::connect_bus_and_stop(size_t bus_index, size_t stop_index)
{
  buses_to_stops_[bus_index].push_back(stop_index);
  if (!stops_to_buses_[stop_index].count(bus_index))
  {
    stops_to_buses_[stop_index].insert(bus_index);
  }
}

void
TransportCatalogue::connect_stop_and_route(
  size_t stop_index,
  size_t route_index
)
{
  routes_[route_index].stops.push_back(stop_index);
  stops_to_routes_[stop_index].push_back(route_index);
}

void
TransportCatalogue::clear()
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

void
TransportCatalogue::write_stop_dependency(
  const StopMeta& stopmeta,
  const std::string& stopname
)
{
  for (size_t i = 0; i != stopmeta.dependencies.size(); i += 2)
  {
    string dependency_stopname = {stopmeta.dependencies[i + 1].begin(),
                                  stopmeta.dependencies[i + 1].end()};
    string::size_type sz;
    int dependency_value{
      static_cast<int>(stoi(string(stopmeta.dependencies[i].begin(),
                                   stopmeta.dependencies[i].end()), &sz
      ))
    };

    if (stops_to_stop_distances_[stopname].count(dependency_stopname))
    {
      stops_to_stop_distances_[stopname][dependency_stopname] =
        dependency_value;
    }
    else
    {
      stops_to_stop_distances_[stopname][dependency_stopname] =
        dependency_value;
      stops_to_stop_distances_[dependency_stopname][stopname] =
        dependency_value;
    }
  }
}

vector<string_view>
QueryParser::GetMetadataQueryForAddBus(const string_view& command)
{
  vector<string_view> bus_metadata_query{};
  bus_metadata_query.reserve(DB_COMMAND_QUERY_MIN_LEXEMS_COUNT);
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
        bus_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*iter
          - &*lexem_begin)}
        );
        lexem_begin = iter + 2 < command.end() ? iter + 2
                                               : throw invalid_command_metadata();
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
        bus_metadata_query.push_back({&*lexem_begin,
                                      static_cast<size_t>(&*(command.rbegin())
                                        + 1 - &*lexem_begin)}
        );
      }

      if (*iter == '-' || *iter == '>')
      {
        bus_metadata_query.push_back({&*lexem_begin,
                                      static_cast<size_t>(&*(iter - 1)
                                        - &*lexem_begin)}
        );
        lexem_begin = iter + 2 < command.end() ? iter + 2
                                               : throw invalid_command_metadata();
      }
    }
  }

  if (is_route_need_reversed)
  {
    bus_metadata_query.reserve(bus_metadata_query.size() * 2 - 1);
    for (auto iter = bus_metadata_query.rbegin() + 1;
         iter != bus_metadata_query.rend() - 1; ++iter)
    {
      bus_metadata_query.push_back(*iter);
    }
  }

  return bus_metadata_query;
}

vector<string_view>
QueryParser::GetMetadataQueryForAddStop(const string_view& command)
{
  vector<string_view> stop_metadata_query{};
  stop_metadata_query.reserve(DB_COMMAND_QUERY_MIN_LEXEMS_COUNT);
  string_view::iterator lexem_begin = command.begin();
  bool is_stop_name_getted(false);
  size_t coordinates_writen_count(0);

  for (auto iter = command.begin(); iter < command.end(); ++iter)
  {
    if (!is_stop_name_getted)
    {
      if (*iter == ':')
      {
        is_stop_name_getted = true;
        stop_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*iter
          - &*lexem_begin)}
        );
        lexem_begin = iter + 2 < command.end() ? iter + 2
                                               : throw invalid_command_metadata();
        ++iter;
      }
    }
    else if (coordinates_writen_count != 2)
    {
      if (*iter == ',' || command.end() - iter == 1)
      {
        if (command.end() - iter == 1)
        {
          ++iter;
        }

        ++coordinates_writen_count;
        stop_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(iter
          - lexem_begin)}
        );

        if (command.end() - iter == 0)
        {
          break;
        }

        lexem_begin = iter + 2 < command.end() ? iter + 2
                                               : throw invalid_command_metadata();
        ++iter;
      }
    }
    else
    {
      if (command.end() - iter == 1)
      {
        stop_metadata_query.push_back({&*lexem_begin,
                                       static_cast<size_t>(&*(command.rbegin())
                                         + 1 - &*lexem_begin)}
        );
        break;
      }

      if (command.end() - iter > 5 && (string_view(&*iter, 5) == "m to "sv))
      {
        stop_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*iter
          - &*lexem_begin)}
        );
        lexem_begin = iter + 5;
        iter += 4;
      }

      if (*iter == ',')
      {
        stop_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*iter
          - &*lexem_begin)}
        );
        lexem_begin = iter + 2 < command.end() ? iter + 2
                                               : throw invalid_command_metadata();
        ++iter;
      }
    }
  }

  return stop_metadata_query;
}

vector<string_view>
QueryParser::GetMetadataQueryForPrintBus(const string_view& command)
{
  return {command};
}

std::vector<string_view>
QueryParser::GetMetadataQueryForPrintStop(const string_view& command)
{
  return {command};
}

pair<DBCommands, string_view>
QueryParser::GetDBCommandCodeAndQuery(const string& from)
{
  if (from.empty())
  {
    throw empty_command_metadata();
  }

  pair<string_view, string_view>
    command_key_and_meta = DivideCommandByCodeAndValue(from);
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

pair<OutputCommands, string_view>
QueryParser::GetOutputCommandCodeAndQuery(const string& from)
{
  if (from.empty())
  {
    throw empty_command_metadata();
  }

  pair<string_view, string_view>
    command_key_and_meta = DivideCommandByCodeAndValue(from);
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

pair<string_view, string_view>
QueryParser::DivideCommandByCodeAndValue(const string& src)
{
  string_view command_code{};
  string_view command_meta{};

  for (size_t i = 0; i != src.size(); ++i)
  {
    if (src[i] == ' ')
    {
      command_code = {src.data(), i};
      command_meta = {src.data() + i + 1, src.size() - i - 1};
      break;
    }
  }

  return {command_code, command_meta};
}
