#pragma once
#include <string>

// Global Logging Mutex to protect file writes for LLVacController components
void LogMessage(const std::string &processName, const std::string &msg);
