#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Arm.h"
#include "Scheduler.h"
#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char *argv[]) {
  int totalWafers = 10;
  if (argc > 1) {
    totalWafers = std::stoi(argv[1]);
  }

  std::cout << "Starting ATMController... Wafers to process: " << totalWafers
            << std::endl;

  // Connect via TCP
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  Scheduler::g_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverAddr.sin_port = htons(54321);

  while (connect(Scheduler::g_Socket, (sockaddr *)&serverAddr,
                 sizeof(serverAddr)) == SOCKET_ERROR) {
    std::cout << "Connecting to LLVacController Server..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "Connected to LLVacController!" << std::endl;

  // Start TCP Listener Thread
  std::thread listener(Scheduler::TCPListenerTask);

  // Start Scheduler Thread
  std::thread scheduler(Scheduler::RunTask, totalWafers);

  // Distribute total wafers across the 3 arms (Robot 1 has 2 arms, Robot 2 has
  // 1 arm)
  int w1 = totalWafers / 3 + (totalWafers % 3 > 0 ? 1 : 0);
  int w2 = totalWafers / 3 + (totalWafers % 3 > 1 ? 1 : 0);
  int w3 = totalWafers / 3;

  Arm atm1_arm1(1, 1, w1, "recipe1.lcp");
  Arm atm1_arm2(1, 2, w2, "recipe2.lcp");
  Arm atm2_arm1(2, 1, w3, "recipe1.lcp");

  std::thread t1(&Arm::Run, &atm1_arm1);
  std::thread t2(&Arm::Run, &atm1_arm2);
  std::thread t3(&Arm::Run, &atm2_arm1);

  t1.join();
  t2.join();
  t3.join();

  // The scheduler will exit after all wafers are processed
  scheduler.join();

  std::cout << "All wafers processed. ATMController exiting." << std::endl;
  closesocket(Scheduler::g_Socket);
  WSACleanup();

  // Force exit as listener might be blocked on recv
  exit(0);
}
