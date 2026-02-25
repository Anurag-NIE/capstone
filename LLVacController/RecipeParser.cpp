#include "RecipeParser.h"
#include "Logger.h"
#include <fstream>

std::vector<RecipeStep> RecipeParser::Parse(const std::string &recipeName) {
  std::vector<RecipeStep> steps;
  std::string filepath = "C:\\Capstone\\recipes\\" + recipeName;
  std::ifstream file(filepath);
  if (!file.is_open()) {
    LogMessage("LoadlockProcess", "Could not read recipe file: " + filepath);
    return steps;
  }

  std::string line;
  RecipeStep currentStep;
  bool inStep = false;

  while (std::getline(file, line)) {
    if (line.find("Step Description:") != std::string::npos) {
      if (inStep)
        steps.push_back(currentStep);
      currentStep = RecipeStep();
      inStep = true;
    }
    if (inStep) {
      if (line.find("N2 Purge: Yes") != std::string::npos)
        currentStep.n2Purge = true;
      if (line.find("DIW Purge: Yes") != std::string::npos)
        currentStep.diwPurge = true;
      if (line.find("Time(seconds):") != std::string::npos) {
        size_t pos = line.find(":") + 1;
        currentStep.durationSecs = std::stoi(line.substr(pos));
      }
    }
  }
  if (inStep)
    steps.push_back(currentStep);

  if (steps.size() > 10)
    steps.resize(10); // Enforce max 10
  return steps;
}
