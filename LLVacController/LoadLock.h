#pragma once
#include <atomic>
#include <string>
#include <Windows.h>

// Pointers to the dynamically loaded DLL functions
typedef bool (*GetDIFunc)(int);
typedef void (*SetDOFunc)(int, bool);

extern GetDIFunc GetDI;
extern SetDOFunc SetDO;

// Hardware IO Enums
enum class DI {
  VacRobotWaferSensor = 4,
  LLDoorStatus = 5,
  LLSlitStatus = 6,
  LLN2PurgeStatus = 7,
  LLDIWPurgeStatus = 8
};

enum class DO {
  LLDoorOpenClose = 0,
  LLSlitOpenClose = 1,
  LLN2PurgeStartStop = 2,
  LLDIWPurgeStartStop = 3
};

class LoadLock {
public:
  std::atomic<bool> waferPresent{false};
  std::atomic<bool> processing{false};

  // Computes state against DO/DI
  bool IsAvailable();

  // Runs a recipe logic on the LoadLock Hardware Thread
  void ExecuteRecipe(const std::string &recipeName);
};

// Global LL instance accessible to main
extern LoadLock g_LoadLock;
