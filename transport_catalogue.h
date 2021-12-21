#pragma once

#include <map>
#include <vector>
#include <optional>
#include <string>
#include <exception>

#include "geo.h"

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
  std::string name;
  Coordinates coordinates;
};

struct bus
{
  size_t id;
};

using Route = std::vector<size_t>;
using BusMeta = std::pair<size_t, std::vector<std::string_view>>;
using StopMeta = std::pair<std::string, Coordinates>;

class TransportCatalogue
{
public:
  struct not_closed_route_exception : public std::exception
  {};
  struct invalid_command : public std::exception
  {};
  struct invalid_command_code : public std::exception
  {};
  struct invalid_command_metadata : public std::exception
  {};
  struct empty_command_metadata : public std::exception
  {};
private:
  /**
   * @brief Здесь хранятся исходники маршрутов.
   */
  std::vector<Route> routes_;
  /**
   * @brief Здесь хранятся исходники остановок.
   */
  std::vector<stop> stops_;
  /**
   * @brief Здесь хранятся исходники автобусов.
   */
  std::vector<bus> buses_;
  /**
   * @brief Карта для нахождения остановки по имени. Если остановка запрашивается перед ее созданием, но optional пуст
   */
  std::map<std::string_view, std::optional<size_t>> names_to_stops_;
  /**
   * @brief Карта для связи остановки с автобусами
   */
  std::map<size_t, std::vector<size_t>> stops_to_buses_;
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
  std::map<size_t, size_t> ids_to_buses_;
public:
  /**
   * @brief Применяет команду к целевому справочнику
   * @param command Код команды с соответствующими данными
   */
  void apply_db_command(const std::string &command);
  /**
   * @brief Применяет команду к целевому справочнику
   * @param command Код команды с соответствующими данными
   */
  void apply_output_command(const std::string &command);
private:
  /**
   * @brief Проверяет, что маршрут является замкнутым
   * @param bus Автобус, маршрут которого будет проверен
   * @return true, если первая остановка встречается в маршруте два раза на первой и последней позиции
   */
  static bool IsClosedRoute(const Route *route);
  /**
   * @brief Разбирает исходную строку команды на ключевые слова, которые влияют на последующую обработку
   * @param code Число из @enum DBCommands или из @enum OutputCommands
   * @param command Исходный текст метаданных, в нем отсутствует код команды
   * @return Вектор подстрок из ключевых слов, на основе которых потом будут генерироваться структуры
   * @throw @struct invalid_command_metadata если в тексте метаданных содержится ошибка
   * @throw @struct invalid_command_code если код команды, не может сматчится ни с одним из обрабатываемых перечислений
   */
  static std::vector<std::string_view> GetMetadataQueryByCode(size_t code, const std::string_view &command);

  static std::vector<std::string_view> GetMetadataQueryForAddBus(const std::string_view &command);
  static std::vector<std::string_view> GetMetadataQueryForAddStop(const std::string_view &command);
  static std::vector<std::string_view> GetMetadataQueryForPrintBus(const std::string_view &command);
  static std::vector<std::string_view> GetMetadataQueryForPrintStop(const std::string_view &command);

  /**
   * @brief Возвращает тип команды, а также подстроку, содержащую метаданные по ней для дальнейшей конвертации
   * @param from Исходный текст команды с ключом и метаданными
   * @return Пара из кода команды, и подстроки с метаданными
   * @throw @struct invalid_command если в тексте команды содержится ошибка
   */
  static std::pair<DBCommands, std::string_view> GetDBCommandCodeAndQuery(const std::string &from);
  /**
   * @brief Возвращает тип команды, а также подстроку, содержащую метаданные по ней для дальнейшей конвертации
   * @param from Исходный текст команды с ключом и метаданными
   * @return Пара из кода команды, и подстроки с метаданными
   * @throw @struct invalid_command если в тексте команды содержится ошибка
   */
  static std::pair<OutputCommands, std::string_view> GetOutputCommandCodeAndQuery(const std::string &from);
  /**
   * @brief Собирает структуру автобуса на основе соответствующих метаданных
   * @param bus_meta Данные, необходимые для конструирования структуры
   * @return Экземпляр автобуса
   */
  static BusMeta MakeBusMetaFrom(std::string_view meta_query);

  static StopMeta MakeStopMetaFrom(std::string_view meta_query);

  static bus BuildBusFrom(BusMeta bus_meta);
  /**
   * @brief Собирает маршрут из остановок
   * @param stop_meta Данные, необходимые для конструирования остановки
   * @return Экземпляр маршрута
   */
  static stop BuildStopFrom(StopMeta stop_meta);
  /**
   * @brief Разделяет строку на часть, связанную с названием команды и на часть, связанную с метаданными
   * @param src целевая строка
   * @return Пара из ключа команды и остальной строки
   */
  static std::pair<std::string_view, std::string_view> DivideCommandByCodeAndValue(const std::string &src);

  void add_stop(const stop &&stop);

  void add_bus(const bus &&bus);

  void add_route(const Route &&route);

  void connect_bus_and_stop(size_t bus_index, size_t stop_index);
  void connect_stop_and_route(size_t stop_index, size_t route_index);

  /**
   * @brief Удаляет остановку из хранилища, а так же все указатели на нее
   * @param stop_index Индекс остановки в соответствующем векторе
   */
  void remove_stop(size_t stop_index);

  /**
   * @brief Удаляет автобус и все указатели на него
   * @param bus_index Индекс автобуса в соответствующем векторе
   */
  void remove_bus(size_t bus_index);

  /**
   * @brief Удаление маршрута, а так же всех упоминаний о нем
   * @param route_index Индекс маршрута в соответствующем векторе
   */
  void remove_route(size_t route_index);

  void clear();
};
