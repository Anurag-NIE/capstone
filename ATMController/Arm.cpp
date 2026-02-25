#include "Arm.h"
#include "Logger.h"
#include "Scheduler.h"
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

Arm::Arm(int rId, int aId, int wProd, std::string recipe)
    : robotId(rId), armId(aId), wafersToProduce(wProd), defaultRecipe(recipe) {}

void Arm::Run() {
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> dist(0, 10); // 0-10 seconds

  for (int i = 0; i < wafersToProduce; i++) {
    // Simulate random production time
    std::this_thread::sleep_for(std::chrono::seconds(dist(rng)));

    LogMessage("AtmRobot", "Wafer Created at Atm Robot " +
                               std::to_string(robotId) + " Arm " +
                               std::to_string(armId));

    // Queue it
    {
      std::lock_guard<std::mutex> lock(Scheduler::g_queueMutex);
      Scheduler::g_waferQueue.push({robotId, armId, defaultRecipe});
    }
    Scheduler::g_queueCV.notify_all(); // Wake up scheduler
  }
}
