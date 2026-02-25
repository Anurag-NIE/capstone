#pragma once

// DLL Export/Import macros
#ifdef HARDWAREIO_EXPORTS
#define HARDWAREIO_API __declspec(dllexport)
#else
#define HARDWAREIO_API __declspec(dllimport)
#endif

// Define enums for IO mapping as per the specification
enum class DI {
    AtmRobot1Arm1WaferSensor = 0,
    AtmRobot1Arm2WaferSensor = 1,
    AtmRobot2WaferSensor = 2,
    VacRobotWaferSensor = 4,
    LLDoorStatus = 5,
    LLSlitStatus = 6,
    LLN2PurgeStatus = 7,
    LLDIWPurgeStatus = 8
};

enum class DO {
    LLDoorOpenClose = 0,
    LLSlitOpenClose = 1,
    LLN2PurgeStartStop = 2,
    LLDIWPurgeStartStop = 3
};

extern "C" {
    // Core APIs specified in the document
    HARDWAREIO_API bool GetDI(int ioNum);
    HARDWAREIO_API void SetDO(int ioNum, bool bValue);
}
