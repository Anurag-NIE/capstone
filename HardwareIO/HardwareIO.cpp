#include "HardwareIO.h"
#include <mutex>

// Define EXPORTS macro so the functions get exported when compiling this DLL
#define HARDWAREIO_EXPORTS

// Global mutex to make IO access thread-safe
std::mutex g_ioMutex;

// State arrays to simulate actual hardware IO lines in debug mode
// DIs usually represent sensor inputs (0-15 typical range)
// DOs usually represent actuator outputs (0-15 typical range)
static bool g_diState[16] = {false};
static bool g_doState[16] = {false};

extern "C" {

// API to get Digital Input status
HARDWAREIO_API bool GetDI(int ioNum) {
  std::lock_guard<std::mutex> lock(g_ioMutex);

#ifdef _DEBUG
  // Debug mode: simulation.
  // As per requirements: if DO DoorOpen = true -> DI DoorStatus = true, etc.
  // We handle this linkage internally inside SetDO to keep GetDI fast.

  if (ioNum >= 0 && ioNum < 16) {
    return g_diState[ioNum];
  }
  return false;
#else
  // Release mode: placeholder for real hardware calls.
  // e.g., return RealHardware::ReadDigitalInput(ioNum);

  // Return simulated state anyway to prevent compilation errors and allow basic
  // testing
  if (ioNum >= 0 && ioNum < 16) {
    return g_diState[ioNum];
  }
  return false;
#endif
}

// API to set Digital Output status
HARDWAREIO_API void SetDO(int ioNum, bool bValue) {
  std::lock_guard<std::mutex> lock(g_ioMutex);

#ifdef _DEBUG
  // Debug mode: simulation.
  if (ioNum >= 0 && ioNum < 16) {
    g_doState[ioNum] = bValue;

    // Link DO commands to DI sensors automatically as per spec
    if (ioNum == (int)DO::LLDoorOpenClose) {
      g_diState[(int)DI::LLDoorStatus] = bValue;
    } else if (ioNum == (int)DO::LLSlitOpenClose) {
      g_diState[(int)DI::LLSlitStatus] = bValue;
    } else if (ioNum == (int)DO::LLN2PurgeStartStop) {
      g_diState[(int)DI::LLN2PurgeStatus] = bValue;
    } else if (ioNum == (int)DO::LLDIWPurgeStartStop) {
      g_diState[(int)DI::LLDIWPurgeStatus] = bValue;
    }
  }
#else
  // Release mode: placeholder for real hardware calls.
  // e.g., RealHardware::WriteDigitalOutput(ioNum, bValue);

  // Maintain state to prevent errors
  if (ioNum >= 0 && ioNum < 16) {
    g_doState[ioNum] = bValue;

    if (ioNum == (int)DO::LLDoorOpenClose)
      g_diState[(int)DI::LLDoorStatus] = bValue;
    if (ioNum == (int)DO::LLSlitOpenClose)
      g_diState[(int)DI::LLSlitStatus] = bValue;
    if (ioNum == (int)DO::LLN2PurgeStartStop)
      g_diState[(int)DI::LLN2PurgeStatus] = bValue;
    if (ioNum == (int)DO::LLDIWPurgeStartStop)
      g_diState[(int)DI::LLDIWPurgeStatus] = bValue;
  }
#endif
}
}
