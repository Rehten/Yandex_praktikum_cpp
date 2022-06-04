#pragma once

#include "transport_catalogue.h"

#define __HAS_JSON_SUPPORT__ 1

#if __HAS_JSON_SUPPORT__
#include "json_reader.h"
#endif

class TransportCatalogue;

struct invalid_command;
struct invalid_command_code;
struct invalid_command_metadata;
struct empty_command_metadata;
struct stop_already_has_coordinates;

enum class DBCommands;
enum class OutputCommands;

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

#if __HAS_JSON_SUPPORT__
class JSONRequestHandler : public RequestHandler
{
  JSONReader json_reader_;

  std::string dcq_from_json(const json::Dict&) noexcept;
  std::string get_raw_db_bus_command_from(const json::Dict&) noexcept;
  std::string get_raw_db_stop_command_from(const json::Dict&) noexcept;
  std::string ocq_from_json(const json::Dict&) noexcept;
  std::string get_raw_output_bus_command_from(const json::Dict&) noexcept;
  std::string get_raw_output_stop_command_from(const json::Dict&) noexcept;
 public:
  std::vector<DBCommandQuery>
  get_db_commands_from(std::istream&) override;
  std::vector<OutputCommandQuery>
  get_output_commands_from(std::istream&) override;
};
#endif


class ResponseSeller
{
 public:
  virtual void
  send_bus(TransportCatalogue*, std::ostream&, std::vector<std::string_view>) = 0;
  virtual void
  send_stop(TransportCatalogue*, std::ostream&, std::vector<std::string_view>) = 0;
};

class RawResponseSeller : public ResponseSeller
{
 public:
  void
  send_bus(TransportCatalogue*, std::ostream&, std::vector<std::string_view>) override;
  void
  send_stop(TransportCatalogue*, std::ostream&, std::vector<std::string_view>) override;
};

#if __HAS_JSON_SUPPORT__
static std::vector<size_t> RequestsIDsList;
static size_t RenderedRequestIndex;

class JSONResponseSeller : public ResponseSeller
{
 public:
  static void
  ApplyRenderStart(std::ostream&);
  static void
  ApplyRenderBetween(std::ostream&);
  static void
  ApplyRenderEnd(std::ostream&);
  void
  send_bus(TransportCatalogue*, std::ostream&, std::vector<std::string_view>) override;
  void
  send_stop(TransportCatalogue*, std::ostream&, std::vector<std::string_view>) override;
};
#endif
