#pragma once
#include <string>
#include <vector>

struct RecipeStep {
  std::string stepDesc;
  bool n2Purge = false;
  bool diwPurge = false;
  int durationSecs = 0;
};

class RecipeParser {
public:
  static std::vector<RecipeStep> Parse(const std::string &recipeName);
};
