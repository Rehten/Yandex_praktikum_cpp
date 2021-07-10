#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <algorithm>

using namespace std;

vector<string> split_into_words(const string& text) {
  vector<string> words;
  string word;
  for (const char c : text) {
    if (c == ' ') {
      words.push_back(word);
      word = "";
    } else {
      word += c;
    }
  }
  words.push_back(word);

  return words;
}

enum class QueryType
{
  NewBus,
  BusesForStop,
  StopsForBus,
  AllBuses,
};

struct Query
{
  QueryType type;
  string bus;
  string stop;
  vector<string> stops;
};

istream &operator>>(istream &is, Query &q)
{
  string user_command{};

  getline(is, user_command);

  vector<string> user_command_query = split_into_words(user_command);

  if (user_command_query.empty())
  {
    return is;
  }

  if (user_command_query[0] == "NEW_BUS"s)
  {
    q.type = QueryType::NewBus;
    q.bus = user_command_query[1];
    q.stops = vector<string>({user_command_query.begin() + 3, user_command_query.end()});
  }
  else if (user_command_query[0] == "BUSES_FOR_STOP"s)
  {
    q.type = QueryType::BusesForStop;
    q.stop = user_command_query[1];
  }
  else if (user_command_query[0] == "STOPS_FOR_BUS"s)
  {
    q.type = QueryType::StopsForBus;
    q.bus = user_command_query[1];
  }
  else if (user_command_query[0] == "ALL_BUSES"s)
  {
    q.type = QueryType::AllBuses;
  }

  return is;
}

struct BusesForStopResponse
{
  string stop;
  vector<string> buses;
};

ostream &operator<<(ostream &os, const BusesForStopResponse &r)
{
  if (r.buses.empty())
  {
    os << "No stop"s;

    return os;
  }

  for (auto bus_iterator = r.buses.begin(); bus_iterator != r.buses.end(); ++bus_iterator)
  {
    if (bus_iterator != r.buses.begin())
    {
      os << ' ';
    }
    os << *bus_iterator;
  }

  return os;
}

struct StopsForBusResponse
{
  string bus;
  vector<pair<string, vector<string>>> stops_to_buses;
};

ostream &operator<<(ostream &os, const StopsForBusResponse &r)
{
  if (!r.bus.size() || r.stops_to_buses.empty())
  {
    os << "No bus"s;

    return os;
  }

  for (const auto &[stop, buses] : r.stops_to_buses)
  {
    os << "Stop "s << stop << ':';

    if (buses.size() == 1 && buses[0] == r.bus)
    {
      os << " no interchange";
    }
    else
    {
      for (const auto &stop_bus : buses)
      {
        if (stop_bus != r.bus)
        {
          os << " " << stop_bus;
        }
      }
    }

    if (stop != r.stops_to_buses.rbegin()->first)
    {
      os << endl;
    }
  }

  return os;
}

struct AllBusesResponse
{
  map<string, vector<string>> buses_to_stops;
};

ostream &operator<<(ostream &os, const AllBusesResponse &r)
{
  if (r.buses_to_stops.empty())
  {
    os << "No buses"s;

    return os;
  }

  for (const auto &[bus, stops] : r.buses_to_stops)
  {
    os << "Bus "s << bus << ':';

    for (const auto &stop : stops)
    {
      os << ' ' << stop;
    }

    if (bus != r.buses_to_stops.rbegin()->first)
    {
      os << endl;
    }
  }

  return os;
}

class BusManager
{
private:
  map<string, vector<string>> buses_to_stops_;
  map<string, vector<string>> stops_to_buses_;
public:
  void AddBus(const string &bus, const vector<string> &stops)
  {
    buses_to_stops_[bus] = stops;

    for (auto &stop : stops)
    {
      stops_to_buses_[stop].push_back(bus);
    }
  }

  BusesForStopResponse GetBusesForStop(const string &stop) const
  {
    vector<string> buses_with_stop = {};

    if (stops_to_buses_.count(stop))
    {
      buses_with_stop = stops_to_buses_.at(stop);
    }

    return { stop, buses_with_stop };
  }

  StopsForBusResponse GetStopsForBus(const string &bus) const
  {
    vector<pair<string, vector<string>>> stops_to_buses {};

    if (buses_to_stops_.count(bus))
    {
      for (const auto &stop : buses_to_stops_.at(bus))
      {
        stops_to_buses.push_back({stop, {}});

        if (stops_to_buses_.count(stop))
        {
          for (const auto &stop_bus : stops_to_buses_.at(stop))
          {
            stops_to_buses.rbegin()->second.push_back(stop_bus);
          }
        }
      }
    }

    return { bus, stops_to_buses };
  }

  AllBusesResponse GetAllBuses() const
  {
    return { buses_to_stops_ };
  }
};

void test_query_type_new_bus_read()
{
  Query query{};
  istringstream input{};

  input.str("NEW_BUS MyBus 4 Stop1 Stop2 Stop3 Stop4"s);

  input >> query;

  assert(query.type == QueryType::NewBus);
  assert(query.bus == "MyBus"s);
  assert(query.stops.size() == 4);
  assert(query.stops[0] == "Stop1"s);
  assert(query.stops[1] == "Stop2"s);
  assert(query.stops[2] == "Stop3"s);
  assert(query.stops[3] == "Stop4"s);
}

void test_query_type_buses_for_stop_read()
{
  Query query{};
  istringstream input{};

  input.str("BUSES_FOR_STOP MyStop1"s);

  input >> query;

  assert(query.type == QueryType::BusesForStop);
  assert(query.stop == "MyStop1"s);
}

void test_query_type_stops_for_bus_read()
{
  Query query{};
  istringstream input{};

  input.str("STOPS_FOR_BUS MyBus1"s);

  input >> query;

  assert(query.type == QueryType::StopsForBus);
  assert(query.bus == "MyBus1"s);
}

void test_query_type_all_buses_read()
{
  Query query{};
  istringstream input{};

  input.str("ALL_BUSES"s);

  input >> query;

  assert(query.type == QueryType::AllBuses);
}

void test_buses_for_stops_response_read()
{
  BusesForStopResponse response{ "TestStop"s, { "Bus1"s, "Bus2"s } };
  ostringstream output;

  output << response;

  assert(output.str() == "Bus1 Bus2"s);
}

void test_buses_for_stops_response_empty()
{
  BusesForStopResponse response{"MyStop1", {}};
  ostringstream output{};

  output << response;

  assert(output.str() == "No stop"s);
}

void test_stops_for_bus_response_read()
{
  StopsForBusResponse response{
    "Bus2"s,
    {
      {"Stop1"s, {"Bus1"s, "Bus2"s}},
      {"Stop2"s, {"Bus2"s, "Bus3"s}},
      {"Stop3"s, {"Bus2"s, "Bus3"s, "Bus4"s}},
    }
  };
  ostringstream output{};

  output << response;

  assert(
    output.str() == (
      "Stop Stop1: Bus1\n"s
      +
      "Stop Stop2: Bus3\n"s
      +
      "Stop Stop3: Bus3 Bus4"s
    )
  );
}

void test_stops_for_bus_response_no_interchange()
{
  StopsForBusResponse response{
    "Bus2"s,
    {
      {"Stop1"s, {"Bus1"s, "Bus2"s}},
      {"Stop2"s, {"Bus2"s}},
      {"Stop3"s, {"Bus2"s, "Bus3"s, "Bus4"s}},
    }
  };
  ostringstream output{};

  output << response;

  auto a = output.str();

  assert(
    output.str() == (
      "Stop Stop1: Bus1\n"s
      +
      "Stop Stop2: no interchange\n"s
      +
      "Stop Stop3: Bus3 Bus4"s
    )
  );
}

void test_stops_for_bus_response_empty()
{
  StopsForBusResponse response{"MyBus1", {}};
  ostringstream output{};

  output << response;

  assert(output.str() == "No bus"s);
}

void test_all_buses_response_read()
{
  AllBusesResponse response{
    {
      {"MyBus1", {"Stop1"s, "Stop2"s}},
      {"MyBus2", {"Stop2"s, "Stop3"s}},
      {"MyBus3", {"Stop3"s, "Stop4"s}},
    }
  };
  ostringstream output{};

  output << response;

  assert(
    output.str() == (
        "Bus MyBus1: Stop1 Stop2\n"s
        +
        "Bus MyBus2: Stop2 Stop3\n"s
        +
        "Bus MyBus3: Stop3 Stop4"s
      )
    );
}

void test_all_buses_response_empty()
{
  AllBusesResponse response{};
  ostringstream output{};

  output << response;

  assert(output.str() == "No buses"s);
}

//int main()
//{
//  test_query_type_new_bus_read();
//  test_query_type_buses_for_stop_read();
//  test_query_type_stops_for_bus_read();
//  test_query_type_all_buses_read();
//
//  test_buses_for_stops_response_read();
//  test_buses_for_stops_response_empty();
//
//  test_stops_for_bus_response_read();
//  test_stops_for_bus_response_empty();
//
//  test_all_buses_response_read();
//  test_all_buses_response_empty();
//}

// Не меняя тела функции main, реализуйте функции и классы выше

int main()
{
  test_query_type_new_bus_read();
  test_query_type_buses_for_stop_read();
  test_query_type_stops_for_bus_read();
  test_query_type_all_buses_read();

  test_buses_for_stops_response_read();
  test_buses_for_stops_response_empty();

  test_stops_for_bus_response_read();
  test_stops_for_bus_response_no_interchange();
  test_stops_for_bus_response_empty();

  test_all_buses_response_read();
  test_all_buses_response_empty();

  int query_count;
  Query q;

  cin >> query_count;

  BusManager bm;
  for (int i = 0; i < query_count; ++i)
  {
    cin >> q;
    switch (q.type)
    {
      case QueryType::NewBus:
        bm.AddBus(q.bus, q.stops);
        break;
      case QueryType::BusesForStop:
        cout << bm.GetBusesForStop(q.stop) << endl;
        break;
      case QueryType::StopsForBus:
        cout << bm.GetStopsForBus(q.bus) << endl;
        break;
      case QueryType::AllBuses:
        cout << bm.GetAllBuses() << endl;
        break;
    }
  }
}
