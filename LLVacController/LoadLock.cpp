#include "LoadLock.h"
#include "Logger.h"
#include "RecipeParser.h"
#include <chrono>
#include <thread>

// Define Globals
GetDIFunc GetDI = nullptr;
SetDOFunc SetDO = nullptr;

LoadLock g_LoadLock;

bool LoadLock::IsAvailable() {
  return !waferPresent && GetDI((int)DI::LLDoorStatus) == false &&
         GetDI((int)DI::LLSlitStatus) == false && !processing;
}

void LoadLock::ExecuteRecipe(const std::string &recipeName) {
  processing = true;
  std::vector<RecipeStep> steps = RecipeParser::Parse(recipeName);

  LogMessage("LoadlockProcess", "LL door closed");
  LogMessage("LoadlockProcess", "Recipe " + recipeName + " started, " +
                                    std::to_string(steps.size()) + " steps");

  int stepNum = 1;
  for (const auto &step : steps) {
    LogMessage("LoadlockProcess",
               "Recipe " + recipeName + ": Step " + std::to_string(stepNum) +
                   " started: " + std::to_string(step.durationSecs) + " secs");

    if (step.n2Purge) {
      SetDO((int)DO::LLN2PurgeStartStop, true);
      LogMessage("LoadlockProcess", "Recipe " + recipeName + ": Step " +
                                        std::to_string(stepNum) +
                                        ": N2 Purge started");
    } else if (step.diwPurge) {
      SetDO((int)DO::LLDIWPurgeStartStop, true);
      LogMessage("LoadlockProcess", "Recipe " + recipeName + ": Step " +
                                        std::to_string(stepNum) +
                                        ": DIW Purge started");
    }

    std::this_thread::sleep_for(std::chrono::seconds(step.durationSecs));

    if (step.n2Purge) {
      SetDO((int)DO::LLN2PurgeStartStop, false);
      LogMessage("LoadlockProcess", "Recipe " + recipeName + ": Step " +
                                        std::to_string(stepNum) +
                                        ": N2 Purge ended");
    } else if (step.diwPurge) {
      SetDO((int)DO::LLDIWPurgeStartStop, false);
      LogMessage("LoadlockProcess", "Recipe " + recipeName + ": Step " +
                                        std::to_string(stepNum) +
                                        ": DIW Purge ended");
    }

    LogMessage("LoadlockProcess", "Recipe " + recipeName + ": Step " +
                                      std::to_string(stepNum) + " ended");
    stepNum++;
  }

  LogMessage("LoadlockProcess", "Recipe " + recipeName + " ended");
  processing = false;
}
