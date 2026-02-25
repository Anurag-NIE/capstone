#include "Logger.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <ctime>
#include <direct.h>

// Global Logging Mutex to protect file writes
std::mutex g_logMutex;

void LogMessage(const std::string &processName, const std::string &msg) {
  std::lock_guard<std::mutex> lock(g_logMutex);

  // Ensure logs directory exists
  _mkdir("C:\\Capstone");
  _mkdir("C:\\Capstone\\logs");

  // Generate mmddyy
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%m%d%y", &tm);

  // Generate ms precision time block
  auto now = std::chrono::system_clock::now();
  t = std::chrono::system_clock::to_time_t(now);
  auto duration = now.time_since_epoch();
  auto ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() %
      1000;
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&t));

  std::string timestamp =
      std::string(timeStr) + "." + std::to_string(ms) + "  " + msg + "\n";
  std::cout << "[" << processName << "] " << timestamp; // console

  std::string logFilePath = "C:\\Capstone\\logs\\" + processName + "_" +
                            std::string(dateStr) + "_1.log";

  // Check size and rollover (max 128kb)
  std::ifstream checkFile(logFilePath, std::ios::ate | std::ios::binary);
  int kb = checkFile.is_open() ? (int)checkFile.tellg() / 1024 : 0;
  checkFile.close();

  if (kb >= 128) {
    logFilePath = "C:\\Capstone\\logs\\" + processName + "_" +
                  std::string(dateStr) + "_2.log";
  }

  std::ofstream logFile(logFilePath, std::ios_base::app);
  if (logFile.is_open()) {
    logFile << timestamp;
    logFile.close();
  }
}
