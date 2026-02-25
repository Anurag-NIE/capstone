#pragma once
#include <string>

// A struct tracking the intention of an ATM Robot.
struct WaferRequest {
  int robotId;
  int armId;
  std::string recipeName;
};

// Represents an individual Robot Arm inside the ATM Controller
class Arm {
public:
  int robotId;
  int armId;
  int wafersToProduce;
  std::string defaultRecipe;

  Arm(int rId, int aId, int wProd, std::string recipe);

  // Main thread execution logic loop to generate random wafers
  void Run();
};
