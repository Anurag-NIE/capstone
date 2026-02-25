#include "Scheduler.h"
#include "Logger.h"
#include <iostream>
#include <string>
#include <thread>

std::queue<WaferRequest> Scheduler::g_waferQueue;
std::mutex Scheduler::g_queueMutex;
std::condition_variable Scheduler::g_queueCV;

std::atomic<bool> Scheduler::g_llAvailable{false};
std::mutex Scheduler::g_llMutex;
std::condition_variable Scheduler::g_llCV;

SOCKET Scheduler::g_Socket = INVALID_SOCKET;

void Scheduler::TCPListenerTask() {
  char buffer[256];
  int bytesReceived;
  while ((bytesReceived = recv(g_Socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
    buffer[bytesReceived] = '\0';
    std::string msg(buffer);
    if (msg.find("LL_AVAILABLE") != std::string::npos) {
      {
        std::lock_guard<std::mutex> lock(g_llMutex);
        g_llAvailable = true;
      }
      g_llCV.notify_one();
    }
  }
}

void Scheduler::RunTask(int totalWafers) {
  int wafersProcessed = 0;

  while (wafersProcessed < totalWafers) {
    WaferRequest currentReq;

    // Wait until queue is not empty AND LL is available
    {
      std::unique_lock<std::mutex> lock(g_queueMutex);
      g_queueCV.wait(
          lock, [] { return !g_waferQueue.empty() && g_llAvailable.load(); });

      currentReq = g_waferQueue.front();
      g_waferQueue.pop();
    }

    // Claim the LL
    g_llAvailable = false;

    LogMessage("AtmRobot", "LL available: Atm Robot " +
                               std::to_string(currentReq.robotId) + " Arm " +
                               std::to_string(currentReq.armId) +
                               ": Wafer put to LL scheduled");

    // Request place via TCP
    std::string reqCmd = "REQUEST_PLACE " + currentReq.recipeName + "\n";
    send(g_Socket, reqCmd.c_str(), (int)reqCmd.length(), 0);

    // Wait for OK_TO_PLACE
    char buffer[256];
    int bytes = recv(g_Socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
      buffer[bytes] = '\0';
      std::string resp(buffer);
      if (resp.find("OK_TO_PLACE") != std::string::npos) {
        // Command door open
        LogMessage("AtmRobot", "LL door open commanded");
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)); // Simulate door opening latency
        LogMessage("AtmRobot", "LL door opened");

        // Extend and Place Wait simulation
        LogMessage("AtmRobot",
                   "Atm Robot " + std::to_string(currentReq.robotId) + " Arm " +
                       std::to_string(currentReq.armId) + " extended to LL");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        LogMessage("AtmRobot", "Wafer Put at LL");

        // Retract
        LogMessage("AtmRobot",
                   "Atm Robot " + std::to_string(currentReq.robotId) + " Arm " +
                       std::to_string(currentReq.armId) + " retracted from LL");

        // Close Door
        LogMessage("AtmRobot", "LL door closed");

        // Notify LLVacController
        const char *doneMsg = "PLACE_DONE\n";
        send(g_Socket, doneMsg, (int)strlen(doneMsg), 0);

        wafersProcessed++;
      }
    }
  }
}
