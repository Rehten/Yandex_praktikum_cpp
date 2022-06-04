#pragma once

#include <map>
#include <set>
#include <vector>
#include <optional>
#include <string>
#include <exception>
#include <variant>
#include <memory>

#include "geo.h"
#include "json_reader.h"

struct invalid_command : public std::exception
{
};
struct invalid_command_code : public std::exception
{
};
struct invalid_command_metadata : public std::exception
{
};
struct empty_command_metadata : public std::exception
{
};
struct stop_already_has_coordinates : public std::exception
{
};

enum DBCommands
{
  AddBus,
  AddStop,
  AddRoute,
};

enum OutputCommands
{
  PrintBus = 100000,
  PrintStop = 100001,
};

struct stop
{
  size_t id;
  std::string name;
  std::optional<Coordinates> coordinates;
};

struct bus
{
  std::string id;
};

struct route
{
  std::vector<size_t> stops;
};
using BusMeta = std::pair<std::string, std::vector<std::string_view>>;

struct StopMeta
{
  std::string name;
  Coordinates coordinates;
  std::vector<std::string_view> dependencies;
};

class QueryParser
{
 public:
  QueryParser() = delete;
  /**
  * @brief Разбирает исходную строку команды на ключевые слова, которые влияют на последующую обработку
  * @param code Число из @enum DBCommands или из @enum OutputCommands
  * @param command Исходный текст метаданных, в нем отсутствует код команды
  * @return Вектор подстрок из ключевых слов, на основе которых потом будут генерироваться структуры
  * @throw @struct invalid_command_metadata если в тексте метаданных содержится ошибка
  * @throw @struct invalid_command_code если код команды, не может сматчится ни с одним из обрабатываемых перечислений
  */
  static std::vector<std::string_view>
  GetMetadataQueryByCode(size_t code, const std::string_view& command);

  static std::vector<std::string_view>
  GetMetadataQueryForAddBus(const std::string_view& command);

  static std::vector<std::string_view>
  GetMetadataQueryForAddStop(const std::string_view& command);

  static std::vector<std::string_view>
  GetMetadataQueryForPrintBus(const std::string_view& command);

  static std::vector<std::string_view>
  GetMetadataQueryForPrintStop(const std::string_view& command);

  /**
   * @brief Возвращает тип команды, а также подстроку, содержащую метаданные по ней для дальнейшей конвертации
   * @param from Исходный текст команды с ключом и метаданными
   * @return Пара из кода команды, и подстроки с метаданными
   * @throw @struct invalid_command если в тексте команды содержится ошибка
   */
  static std::pair<DBCommands, std::string_view>
  GetDBCommandCodeAndQuery(const std::string& from);

  /**
   * @brief Возвращает тип команды, а также подстроку, содержащую метаданные по ней для дальнейшей конвертации
   * @param from Исходный текст команды с ключом и метаданными
   * @return Пара из кода команды, и подстроки с метаданными
   * @throw @struct invalid_command если в тексте команды содержится ошибка
   */
  static std::pair<OutputCommands, std::string_view>
  GetOutputCommandCodeAndQuery(const std::string& from);
  /**
   * @brief Разделяет строку на часть, связанную с названием команды и на часть, связанную с метаданными
   * @param src целевая строка
   * @return Пара из ключа команды и остальной строки
   */
  static std::pair<std::string_view, std::string_view>
  DivideCommandByCodeAndValue(const std::string& src);
};

class RequestHandler
{
 protected:
  using DBCommandQuery = std::pair<DBCommands, std::string>;
  using OutputCommandQuery = std::pair<OutputCommands, std::string>;
 public:
  virtual std::vector<DBCommandQuery>
  get_db_commands_from(std::istream&) = 0;
  virtual std::vector<OutputCommandQuery>
  get_output_commands_from(std::istream&) = 0;
};

class RawRequestHandler : public RequestHandler
{
 public:
  std::vector<DBCommandQuery>
  get_db_commands_from(std::istream&) override;
  std::vector<OutputCommandQuery>
  get_output_commands_from(std::istream&) override;
};

class JSONRequestHandler : public RequestHandler
{
  RawRequestHandler raw_request_handler_;
  JSONReader json_reader_;

  std::string dcq_from_json(const json::Dict&);
  std::string ocq_from_json(const json::Dict&);
 public:
  std::vector<DBCommandQuery>
  get_db_commands_from(std::istream&) override;
  std::vector<OutputCommandQuery>
  get_output_commands_from(std::istream&) override;
};

class TransportCatalogue
{
  size_t last_stop_id_ = 1;
  /**
   * @brief Здесь хранятся исходники маршрутов.
   */
  std::vector<route> routes_;
  /**
   * @brief Здесь хранятся исходники остановок.
   */
  std::vector<stop> stops_;
  /**
   * @brief Здесь хранятся исходники автобусов.
   */
  std::vector<bus> buses_;
  /**
   * @brief Карта для нахождения остановки по имени.
   */
  std::map<std::string, size_t> names_to_stops_;
  /**
   * @brief Карта для связи остановки с автобусами
   */
  std::map<size_t, std::set<size_t>> stops_to_buses_;
  /**
   * @brief Карта для связи автобуса с остановками
   */
  std::map<size_t, std::vector<size_t>> buses_to_stops_;
  /**
   * @brief Карта для связи остановки с маршрутом в котором она находится
   */
  std::map<size_t, std::vector<size_t>> stops_to_routes_;
  /**
   * @brief Карта для связи id автобуса с самим автобусом
   */
  std::map<std::string, size_t> ids_to_buses_;
  /**
   * @brief Карта для указания расстояния между остановками
   */
  std::map<std::string, std::map<std::string, int>> stops_to_stop_distances_;

  std::unique_ptr<RequestHandler> request_handler_ptr_;
 public:
  TransportCatalogue(std::unique_ptr<RequestHandler>&&);

  void
  listen_db_commands_from(std::istream&);

  void
  listen_output_commands_from(std::istream&, std::ostream&);
 private:
  /**
 * @brief Применяет команду к целевому справочнику
 * @param command Код команды с соответствующими данными
 */
  void
  apply_db_command(const std::pair<DBCommands, std::string>&);

  /**
   * @brief Применяет команду к целевому справочнику
   * @param output_stream Поток вывода
   * @param command Код команды с соответствующими данными
   */
  void
  apply_output_command(std::ostream& output_stream, const std::pair<OutputCommands, std::string>&);

  /**
   * @brief Собирает структуру автобуса на основе соответствующих метаданных
   * @param bus_meta Данные, необходимые для конструирования структуры
   * @return Экземпляр автобуса
   */
  static BusMeta
  MakeBusMetaFrom(std::string_view meta_query);

  static StopMeta
  MakeStopMetaFrom(std::string_view meta_query);

  static bus
  BuildBusFrom(BusMeta bus_meta);

  /**
   * @brief Собирает маршрут из остановок
   * @param stop_meta Данные, необходимые для конструирования остановки
   * @return Экземпляр маршрута
   */
  stop
  build_stop_from(StopMeta stop_meta);

  void
  write_stop_dependency(const StopMeta& stop_meta, const std::string& stopname);

  void
  add_stop(const stop&& stop);

  void
  add_bus(const bus&& bus);

  void
  add_route(const route&& route);

  void
  connect_bus_and_stop(size_t bus_index, size_t stop_index);

  void
  connect_stop_and_route(size_t stop_index, size_t route_index);

  void
  clear();
};
