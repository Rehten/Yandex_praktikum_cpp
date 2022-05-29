#pragma once

#include <vector>
#include <string>

std::vector<std::string> get_db_commands(std::istream &is, size_t count);

std::vector<std::string> user_input_db_command();