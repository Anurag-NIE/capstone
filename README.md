# Applied Materials LoadLock Capstone Project

## Project Overview
A C++ control system for semiconductor wafer processing using LoadLock chambers. This project demonstrates:
- Multi-threaded process control
- Inter-process communication (TCP/IP)
- Hardware abstraction layers
- Recipe-based processing
- Resource scheduling and collision avoidance

## System Components

### 1. ATMController (Atmospheric Robot Process)
- **Robot 1**: 2 arms for wafer handling
- **Robot 2**: 1 arm for wafer handling
- Generates wafers at random intervals (0-10 seconds)
- Schedules LoadLock access to prevent collisions
- Communicates via TCP with LLVacController

### 2. LLVacController (LoadLock & Vacuum Robot Process)
- Manages LoadLock chamber state
- Executes recipes (N2 Purge, DIW Purge)
- Controls Vacuum Robot for processed wafer pickup
- TCP server for ATM communication

### 3. HardwareIO.dll (IO Abstraction)
- Digital Input (DI) APIs: GetDI()
- Digital Output (DO) APIs: SetDO()
- Debug mode: Simulated hardware
- Release mode: Real hardware interface

## Directory Structure
```
CapstoneSystem/
├── ATMController/
│   ├── Logger.h/cpp
│   ├── Arm.h/cpp
│   ├── Scheduler.h/cpp
│   └── ATMController.cpp
├── HardwareIO/
│   ├── HardwareIO.h
│   └── HardwareIO.cpp
├── LLVacController/
│   ├── Logger.h/cpp
│   ├── RecipeParser.h/cpp
│   ├── LoadLock.h/cpp
│   └── LLVacController.cpp
├── recipes/
│   ├── recipe1.lcp
│   └── recipe2.lcp
├── run_system.bat
└── SETUP_GUIDE.txt
```

## Quick Start

### Prerequisites
- Visual Studio 2019 or later
- Windows SDK
- C++14 or higher

### Build Instructions
1. Open Visual Studio
2. Create Blank Solution named "CapstoneSystem"
3. Add three projects:
   - HardwareIO (DLL)
   - LLVacController (Console App)
   - ATMController (Console App)
4. Add source files to respective projects
5. Configure linker dependencies (ws2_32.lib for network projects)
6. Build Solution (x64 Debug)

### Run Instructions
```batch
# Method 1: Using batch file
cd x64\Debug
run_system.bat

# Method 2: Manual start
start LLVacController.exe
timeout /t 2
start ATMController.exe 10
```

## Features

### Concurrency & Thread Safety
- **3 Robot Arm Threads**: Generate wafers independently
- **Scheduler Thread**: Coordinates LoadLock access
- **Vacuum Robot Thread**: Asynchronous wafer pickup
- **Mutex Protection**: Thread-safe IO operations
- **Condition Variables**: Efficient waiting/signaling

### Communication Protocol
- **TCP Socket**: Port 54321
- **Messages**:
  - `LL_AVAILABLE` - Server → Client
  - `REQUEST_PLACE [recipe]` - Client → Server
  - `OK_TO_PLACE` - Server → Client
  - `PLACE_DONE` - Client → Server

### Collision Prevention
- Queue-based scheduling with mutex
- Only one arm accesses LoadLock at a time
- Condition variable blocks arms until LL available
- First-detected-wafer-first-served policy

### Recipe Processing
- File format: `.lcp` (key-value pairs)
- Location: `C:\Capstone\recipes\`
- Max 10 steps per recipe
- Supports N2 Purge and DIW Purge
- Configurable duration per step

### Logging System
- Location: `C:\Capstone\logs\`
- Format: `ProcessName_mmddyy_n.log`
- Auto-rollover at 128KB
- Millisecond precision timestamps
- Thread-safe file writes

## Hardware IO Mapping

### Digital Inputs (DI)
| Number | Description |
|--------|-------------|
| 0 | Atm Robot 1 Arm 1 Wafer Sensor |
| 1 | Atm Robot 1 Arm 2 Wafer Sensor |
| 2 | Atm Robot 2 Wafer Sensor |
| 4 | Vacuum Robot Wafer Sensor |
| 5 | LoadLock Door Status |
| 6 | LoadLock Slit Status |
| 7 | LoadLock N2 Purge Status |
| 8 | LoadLock DIW Purge Status |

### Digital Outputs (DO)
| Number | Description |
|--------|-------------|
| 0 | LoadLock Door Open/Close |
| 1 | LoadLock Slit Open/Close |
| 2 | LoadLock N2 Purge Start/Stop |
| 3 | LoadLock DIW Purge Start/Stop |

## Configuration

### Command Line Arguments
```
ATMController.exe [number_of_wafers]
Default: 10 wafers
Example: ATMController.exe 25
```

### Recipe File Format
```
Recipe Name: [name]
Recipe Description: [description]
Step Description: [step name]
N2 Purge: Yes/No
DIW Purge: Yes/No
Time(seconds): [duration]
```

## Log Output Examples

### AtmRobot.log
```
12:35:20.201  Wafer Created at Atm Robot 1 Arm 1
12:35:20.202  LL available: Atm Robot 1 Arm 1: Wafer put to LL scheduled
12:35:20.203  LL door open commanded
12:35:20.304  LL door opened
12:35:20.305  Atm Robot 1 Arm 1 extended to LL
12:35:20.506  Wafer Put at LL
12:35:20.507  Atm Robot 1 Arm 1 retracted from LL
12:35:20.508  LL door closed
```

### LoadlockProcess.log
```
12:35:20.510  LL door closed
12:35:20.511  Recipe recipe1.lcp started, 2 steps
12:35:20.512  Recipe recipe1.lcp: Step 1 started: 2 secs
12:35:20.513  Recipe recipe1.lcp: Step 1: N2 Purge started
12:35:22.515  Recipe recipe1.lcp: Step 1: N2 Purge ended
12:35:22.516  Recipe recipe1.lcp: Step 1 ended
```

### VacuumRobot.log
```
12:35:28.202  Vac Robot available
12:35:28.203  Wafer available at LL: Wafer get scheduled
12:35:28.204  LL Slit Valve Open Commanded
12:35:28.254  LL Slit Valve Opened
12:35:28.255  Vac Robot extended to LL
12:35:28.256  Wafer Get at LL
12:35:28.457  Vac Robot retracted from LL
```

## Troubleshooting

### Common Issues

**Problem**: DLL not found
```
Solution: Copy HardwareIO.dll to same directory as LLVacController.exe
```

**Problem**: Cannot connect to server
```
Solution: Start LLVacController.exe before ATMController.exe
```

**Problem**: Recipe file not found
```
Solution: Ensure recipes exist in C:\Capstone\recipes\
          Run: mkdir C:\Capstone\recipes
          Copy recipe files to that location
```

**Problem**: Access denied to C:\Capstone
```
Solution: Run Visual Studio as Administrator
          OR change log/recipe paths in code
```

## Technical Highlights

### Design Patterns
- **Producer-Consumer**: Arm threads produce, scheduler consumes
- **State Machine**: LoadLock availability management
- **Singleton**: Global LoadLock instance
- **Strategy**: Debug vs Release IO implementations

### C++ Features Used
- `std::thread`, `std::mutex`, `std::condition_variable`
- `std::atomic` for lock-free flags
- `std::queue` for wafer requests
- Lambda functions for detached threads
- Smart pointers and RAII principles

### Windows APIs
- Winsock2: TCP/IP networking
- LoadLibrary/GetProcAddress: Dynamic DLL loading
- _mkdir: Directory creation

## Performance Characteristics
- Wafer generation: 0-10 seconds (random)
- Door operation: ~100ms simulation
- Arm extend/retract: ~200ms simulation
- Recipe execution: Configurable (seconds per step)
- Vacuum pickup: 5 seconds simulation
- Log file I/O: Buffered writes with mutex

## Testing Recommendations
1. **Light Load**: 5 wafers - verify basic flow
2. **Normal Load**: 10-20 wafers - check scheduling
3. **Heavy Load**: 50+ wafers - stress test
4. **Edge Cases**: 1 wafer, 100 wafers
5. **Recipe Variations**: Different step counts and durations

## Project Requirements Met
✅ Two separate processes with different controllers  
✅ IPC using TCP sockets (not shared memory)  
✅ Multi-threading with proper synchronization  
✅ Mutex-protected critical sections  
✅ LoadLock collision prevention  
✅ Recipe file parsing and execution  
✅ Dynamic DLL loading  
✅ Enum-based IO mapping  
✅ Logging with 128KB rollover  
✅ Debug/Release build support  
✅ Command-line argument handling  

## Future Enhancements
- [ ] GUI monitoring dashboard
- [ ] Real-time visualization
- [ ] Performance metrics collection
- [ ] Enhanced error recovery
- [ ] Database logging
- [ ] Remote monitoring capabilities
- [ ] Configuration file support
- [ ] Unit test suite

## License
Educational/Capstone Project - Applied Materials

## Contact
This is a capstone project for Applied Materials training program.
