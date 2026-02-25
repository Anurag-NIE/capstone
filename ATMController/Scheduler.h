#pragma once
#include "Arm.h"
#include <WinSock2.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

class Scheduler {
public:
  static std::queue<WaferRequest> g_waferQueue;
  static std::mutex g_queueMutex;
  static std::condition_variable g_queueCV;

  static std::atomic<bool> g_llAvailable;
  static std::mutex g_llMutex;
  static std::condition_variable g_llCV;

  static SOCKET g_Socket;

  // TCP Client listner thread loop
  static void TCPListenerTask();

  // The main wait/schedule logic for ATM Robot Arms
  static void RunTask(int totalWafers);
};
