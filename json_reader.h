#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "json.h"

class JSONReader
{
  std::vector<std::string> json_raws_;
 public:
  [[nodiscard]]
  json::Document
  get_json_as_document() const;

  friend std::istream&
  operator >>(std::istream&, JSONReader&);

  friend std::ostream&
  operator <<(std::ostream&, JSONReader&);

  explicit operator bool();
};
