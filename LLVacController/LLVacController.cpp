#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "LoadLock.h"
#include "Logger.h"
#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

SOCKET g_ClientSocket = INVALID_SOCKET;

// ----------------------------------------------------
// Vacuum Robot
// ----------------------------------------------------
void VacuumRobotTask() {
  while (true) {
    if (g_LoadLock.waferPresent && !g_LoadLock.processing) {
      LogMessage("VacuumRobot", "Vac Robot available");
      LogMessage("VacuumRobot", "Wafer available at LL: Wafer get scheduled");

      // Open Slit Valve
      SetDO((int)DO::LLSlitOpenClose, true);
      LogMessage("VacuumRobot", "LL Slit Valve Open Commanded");

      // Wait for DI
      while (!GetDI((int)DI::LLSlitStatus))
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      LogMessage("VacuumRobot", "LL Slit Valve Opened");

      // Extend, get, retract
      LogMessage("VacuumRobot", "Vac Robot extended to LL");
      LogMessage("VacuumRobot", "Wafer Get at LL");
      std::this_thread::sleep_for(std::chrono::milliseconds(200));

      g_LoadLock.waferPresent = false; // Wafer removed
      LogMessage("VacuumRobot", "Vac Robot retracted from LL");

      // Close Slit Valve
      SetDO((int)DO::LLSlitOpenClose, false);
      LogMessage("VacuumRobot", "LL Slit Valve Close Commanded");

      while (GetDI((int)DI::LLSlitStatus))
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      LogMessage("VacuumRobot", "LL Slit Valve Closed");

      LogMessage("VacuumRobot", "Wafer put to other process chamber");
      std::this_thread::sleep_for(
          std::chrono::seconds(5)); // Simulate processing off-site

      LogMessage("VacuumRobot", "Vac Robot available");

      // Notify ATMController that LL is available again
      if (g_ClientSocket != INVALID_SOCKET && g_LoadLock.IsAvailable()) {
        const char *msg = "LL_AVAILABLE\n";
        send(g_ClientSocket, msg, (int)strlen(msg), 0);
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

// ----------------------------------------------------
// TCP Server (Listens for ATMController)
// ----------------------------------------------------
void StartTCPServer() {
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);

  SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(54321);

  bind(listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr));
  listen(listenSocket, SOMAXCONN);

  std::cout << "LLVacController listening on port 54321..." << std::endl;

  while (true) {
    g_ClientSocket = accept(listenSocket, NULL, NULL);
    std::cout << "ATMController connected." << std::endl;

    // Immediately send LL_AVAILABLE on first connect if conditions are met
    if (g_LoadLock.IsAvailable()) {
      const char *msg = "LL_AVAILABLE\n";
      send(g_ClientSocket, msg, (int)strlen(msg), 0);
    }

    char buffer[256];
    int bytesReceived;
    while ((bytesReceived =
                recv(g_ClientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
      buffer[bytesReceived] = '\0';
      std::string cmd(buffer);

      if (cmd.find("REQUEST_PLACE") != std::string::npos &&
          g_LoadLock.IsAvailable()) {
        std::string recipeName =
            cmd.substr(14); // e.g. "REQUEST_PLACE xyz.lcp" -> "xyz.lcp"
        // Clean up newline if present
        if (!recipeName.empty() && recipeName.back() == '\n')
          recipeName.pop_back();

        // Send OK
        const char *ack = "OK_TO_PLACE\n";
        send(g_ClientSocket, ack, (int)strlen(ack), 0);

        // Wait for ATM to place
        char placeDone[256];
        int n = recv(g_ClientSocket, placeDone, sizeof(placeDone) - 1, 0);
        if (n > 0) {
          placeDone[n] = '\0';
          std::string pdCmd(placeDone);
          if (pdCmd.find("PLACE_DONE") != std::string::npos) {
            g_LoadLock.waferPresent = true;

            // Execute recipe in a separate thread so network loop isn't blocked
            std::thread([recipeName]() {
              g_LoadLock.ExecuteRecipe(recipeName);
            }).detach();
          }
        }
      }
    }
    closesocket(g_ClientSocket);
    g_ClientSocket = INVALID_SOCKET;
    std::cout << "ATMController disconnected." << std::endl;
  }
}

int main() {
  // Dynamically load HardwareIO DLL
  HMODULE hDll = LoadLibraryA("HardwareIO.dll");
  if (hDll != NULL) {
    GetDI = (GetDIFunc)GetProcAddress(hDll, "GetDI");
    SetDO = (SetDOFunc)GetProcAddress(hDll, "SetDO");
  } else {
    std::cerr << "Failed to load HardwareIO.dll. Ensure it's built and in same directory as executable."
              << std::endl;
    return 1;
  }

  std::cout << "Starting LLVacController..." << std::endl;

  // Start background Vacuum Robot Thread
  std::thread vacThread(VacuumRobotTask);

  // Start blocking TCP Server
  StartTCPServer();

  vacThread.join();
  FreeLibrary(hDll);
  WSACleanup();
  return 0;
}
