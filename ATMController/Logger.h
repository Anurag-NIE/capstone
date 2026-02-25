#pragma once
#include <string>

// Thread-safe logging function mapping to C:\Capstone\logs
void LogMessage(const std::string &processName, const std::string &msg);
