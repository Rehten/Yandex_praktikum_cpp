#include <iostream>
#include <utility>
#include <numeric>
#include <algorithm>
#include <cstring>

#include "transport_catalogue.h"

#define DB_COMMAND_QUERY_MIN_LEXEMS_COUNT 3

using namespace std;
using namespace literals::string_literals;
using namespace literals::string_view_literals;

void TransportCatalogue::apply_db_command(const string &command)
{
  pair<DBCommands, string_view> db_parsed_command = GetDBCommandCodeAndQuery(command);

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

      transform(busmeta.second.begin(), busmeta.second.end(), stops.begin(), [](const string_view &stop_sv) -> string {
        return {stop_sv.begin(), stop_sv.end()};
      });

      for (const string &stopname: stops)
      {
        if (!names_to_stops_.count(stopname))
        {
          add_stop({++last_stop_id_, {stopname.begin(), stopname.end()}, nullopt});
        }
        connect_bus_and_stop(ids_to_buses_.at(busmeta.first), names_to_stops_.at(stopname));
      }

      transform(stops.begin(), stops.end(), added_route.stops.begin(), [this](const string &stopname) -> size_t {
        return names_to_stops_.at(stopname);
      });

      add_route(move(added_route));

      for (size_t stop_index: set(routes_[routes_.size() - 1].stops.begin(), routes_[routes_.size() - 1].stops.end()))
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
      string bus_id = string(query[0].begin(), query[0].end());

      output_stream << "Bus "s << bus_id << ": "s;

      if (!ids_to_buses_.count(bus_id))
      {
        output_stream << "not found"s << endl;
      }
      else
      {
        auto selected_bus_stops = buses_to_stops_.at(ids_to_buses_.at(bus_id));

        if (selected_bus_stops.size() < 2)
        {
          throw invalid_command_metadata();
        }

        size_t routes_count = selected_bus_stops.size();
        size_t unique_routes_count = set(selected_bus_stops.begin(), selected_bus_stops.end()).size();
        bool is_practical_length_can_be_calculated(true);
        double theoretical_routes_length{};
        double practical_routes_length{};

        for (size_t i = 1; i != selected_bus_stops.size(); ++i)
        {
          stop prev = stops_[selected_bus_stops[i - 1]];
          stop cur = stops_[selected_bus_stops[i]];

          theoretical_routes_length += ComputeDistance(*prev.coordinates, *cur.coordinates);

          if (
            is_practical_length_can_be_calculated &&
            stops_to_stop_distances_.count(prev.name)
            &&
            stops_to_stop_distances_.at(prev.name).count(cur.name))
          {
            practical_routes_length += static_cast<double>(
              stops_to_stop_distances_
                .at(prev.name)
                .at(cur.name)
            );
          }
          else
          {
            is_practical_length_can_be_calculated = false;
          }
        }

        output_stream << routes_count << " stops on route, "s
                      << unique_routes_count << " unique stops, "s
                      << (is_practical_length_can_be_calculated ? practical_routes_length : theoretical_routes_length) << " route length, "s
                      << practical_routes_length/theoretical_routes_length << " curvature"s << endl;
      }
    }
      break;
    case OutputCommands::PrintStop:
    {
      string query_stop_name = string(query[0].begin(), query[0].end());

      if (!names_to_stops_.count(query_stop_name))
      {
        output_stream << "Stop " << query_stop_name << ": not found" << endl;
        return;
      }

      size_t stop_index = names_to_stops_.at(query_stop_name);

      if (!stops_to_buses_.count(stop_index) || stops_to_buses_.at(stop_index).empty())
      {
        output_stream << "Stop " << query_stop_name << ": no buses" << endl;
        return;
      }

      auto &buses_indexes = stops_to_buses_.at(names_to_stops_.at(query_stop_name));
      vector<string> buses_ids{};

      buses_ids.reserve(buses_indexes.size());

      output_stream << "Stop " << query_stop_name << ": buses ";
      for (size_t bus_index: buses_indexes)
      {
        buses_ids.push_back(buses_[bus_index].id);
      }

      sort(buses_ids.begin(), buses_ids.end(), less());

      for (const string &id: buses_ids)
      {
        output_stream << id << " ";
      }
      output_stream << endl;
      return;
    }
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
    case OutputCommands::PrintStop:
      return GetMetadataQueryForPrintStop(command);
    default:
      throw invalid_command_code();
  }
}

vector<string_view> TransportCatalogue::GetMetadataQueryForAddBus(const string_view &command)
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
        bus_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*iter - &*lexem_begin)});
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
        bus_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*(command.rbegin()) + 1 - &*lexem_begin)});
      }

      if (*iter == '-' || *iter == '>')
      {
        bus_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*(iter - 1) - &*lexem_begin)});
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
  stop_metadata_query.reserve(DB_COMMAND_QUERY_MIN_LEXEMS_COUNT);
  string_view::iterator lexem_begin = command.begin();
  bool is_stop_name_getted(false);
  size_t coordinates_writen_count(0);

  for (auto iter = command.begin(); iter != command.end(); ++iter)
  {
    if (!is_stop_name_getted)
    {
      if (*iter == ':')
      {
        is_stop_name_getted = true;
        stop_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*iter - &*lexem_begin)});
        lexem_begin = iter + 2 < command.end() ? iter + 2 : throw invalid_command_metadata();
        ++iter;
      }
    }
    else if (coordinates_writen_count != 2)
    {
      if (*iter == ',' || command.end() - iter == 1)
      {
        ++coordinates_writen_count;
        stop_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*iter - &*lexem_begin)});

        if (command.end() - iter == 1)
        {
          break;
        }

        lexem_begin = iter + 2 < command.end() ? iter + 2 : throw invalid_command_metadata();
        ++iter;
      }
    }
    else
    {
      if (command.end() - iter == 1)
      {
        stop_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*(command.rbegin()) + 1 - &*lexem_begin)});
        break;
      }

      if (command.end() - iter > 5 && (string_view(&*iter, 5) == "m to "sv))
      {
        stop_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*iter - &*lexem_begin)});
        lexem_begin = iter + 5;
        iter += 4;
      }

      if (*iter == ',')
      {
        stop_metadata_query.push_back({&*lexem_begin, static_cast<size_t>(&*iter - &*lexem_begin)});
        lexem_begin = iter + 2 < command.end() ? iter + 2 : throw invalid_command_metadata();
        ++iter;
      }
    }
  }

  return stop_metadata_query;
}

vector<string_view> TransportCatalogue::GetMetadataQueryForPrintBus(const string_view &command)
{
  return {command};
}

std::vector<string_view> TransportCatalogue::GetMetadataQueryForPrintStop(const string_view &command)
{
  return {command};
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
  vector<string_view> splitted_meta_query = GetMetadataQueryByCode(DBCommands::AddBus, meta_query);

  return {
    string(
      splitted_meta_query[0].begin(),
      splitted_meta_query[0].end()
    ),
    {splitted_meta_query.begin() + 1, splitted_meta_query.end()}
  };
}

StopMeta TransportCatalogue::MakeStopMetaFrom(string_view meta_query)
{
  vector<string_view> splitted_meta_query = GetMetadataQueryByCode(DBCommands::AddStop, meta_query);

  if (splitted_meta_query.size() < DB_COMMAND_QUERY_MIN_LEXEMS_COUNT)
  {
    throw invalid_command();
  }

  auto dependencies = splitted_meta_query.size() == DB_COMMAND_QUERY_MIN_LEXEMS_COUNT
                      ? vector<string_view>{} : vector<string_view>{splitted_meta_query.begin() + 3,
                                                                    splitted_meta_query.end()};

  return {
    {splitted_meta_query[0].begin(), splitted_meta_query[0].end()},
    Coordinates{
      stod(string{splitted_meta_query[1].begin(), splitted_meta_query[1].end()}),
      stod(string{splitted_meta_query[2].begin(), splitted_meta_query[2].end()})
    },
    dependencies
  };
}

bus TransportCatalogue::BuildBusFrom(BusMeta bus_meta)
{
  return {bus_meta.first};
}

stop TransportCatalogue::build_stop_from(StopMeta stop_meta)
{
  return stop{++last_stop_id_, stop_meta.name, stop_meta.coordinates};
}

pair<string_view, string_view> TransportCatalogue::DivideCommandByCodeAndValue(const string &src)
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

void TransportCatalogue::add_stop(const stop &&stop)
{
  stops_.push_back(stop);
  names_to_stops_[stops_[stops_.size() - 1].name] = stops_.size() - 1;
}

void TransportCatalogue::add_bus(const bus &&bus)
{
  buses_.push_back(bus);
  ids_to_buses_[bus.id] = buses_.size() - 1;
}

void TransportCatalogue::add_route(const route &&route)
{
  routes_.push_back(route);
}

void TransportCatalogue::connect_bus_and_stop(size_t bus_index, size_t stop_index)
{
  buses_to_stops_[bus_index].push_back(stop_index);
  if (!stops_to_buses_[stop_index].count(bus_index))
  {
    stops_to_buses_[stop_index].insert(bus_index);
  }
}

void TransportCatalogue::connect_stop_and_route(size_t stop_index, size_t route_index)
{
  routes_[route_index].stops.push_back(stop_index);
  stops_to_routes_[stop_index].push_back(route_index);
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

void TransportCatalogue::write_stop_dependency(
  const StopMeta &stopmeta,
  const std::string &stopname
)
{
  for (size_t i = 0; i != stopmeta.dependencies.size(); i += 2)
  {
    string dependency_stopname = {stopmeta.dependencies[i + 1].begin(), stopmeta.dependencies[i + 1].end()};
    string::size_type sz;
    int64_t dependency_value{
      static_cast<int64_t>(stoi(string(stopmeta.dependencies[i].begin(), stopmeta.dependencies[i].end()), &sz))
    };

    if (stops_to_stop_distances_[stopname].count(dependency_stopname))
    {
      stops_to_stop_distances_[stopname][dependency_stopname] = dependency_value;
    }
    else
    {
      stops_to_stop_distances_[stopname][dependency_stopname] = dependency_value;
      stops_to_stop_distances_[dependency_stopname][stopname] = dependency_value;
    }
  }
}
