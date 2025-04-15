# Energy-Efficient CPU Scheduler

## Introduction
This project implements an energy-efficient CPU scheduling algorithm designed for mobile and embedded systems. The scheduler optimizes both performance and energy consumption by dynamically adjusting CPU frequency based on process priorities, workload, and completion status.

## Features
- **Dynamic Voltage and Frequency Scaling (DVFS)**
  - Automatically adjusts CPU frequency between 0.5 GHz and 2.0 GHz
  - Considers process priority, workload, and completion status
  - Optimizes energy consumption while maintaining performance

- **Priority-Based Scheduling**
  - Supports priority levels from 1 (highest) to 10 (lowest)
  - Combines priority with remaining execution time for scheduling decisions
  - Ensures fair execution while respecting process priorities

- **Energy Consumption Tracking**
  - Real-time monitoring of energy usage
  - Detailed energy consumption history
  - Energy calculation based on the formula E ∝ f³ * t

- **Comprehensive Statistics**
  - Process-level metrics
    - Turnaround time
    - Waiting time
    - Completion time
    - Individual energy consumption
  - System-level metrics
    - Total energy consumption
    - Average turnaround time
    - Average waiting time
    - Total execution time

## Requirements
- C++ compiler with C++11 support
- Windows or Unix-based operating system
- Minimum 512MB RAM
- Terminal/Console for interface

## Usage
1. Run the compiled program: bash --> g++ -o main main.cpp --> ./main

2. Follow the interactive prompts:
   - Enter time quantum (in milliseconds)
   - Specify number of processes
   - For each process, provide:
     - Burst time (execution time in ms)
     - Arrival time (in ms)
     - Priority (1-10) -->(1 being the highest priority)

3. View Results:
   - Process statistics will be displayed automatically
   - Choose additional options:
     - View energy consumption history
     - Exit program

## Example Usage
--> bash
=== Energy-Efficient CPU Scheduler ===

Enter time quantum for the scheduler (in ms): 2

Enter the number of processes: 3

Process 1 details:
Enter burst time (ms): 6
Enter arrival time (ms): 0
Enter priority (1-10, 1 being highest): 2

Process 2 details:
Enter burst time (ms): 4
Enter arrival time (ms): 1
Enter priority (1-10, 1 being highest): 1

Process 3 details:
Enter burst time (ms): 8
Enter arrival time (ms): 2
Enter priority (1-10, 1 being highest): 3


## Understanding the Output

### Process Statistics

PID  Burst  Arrival  Completion  Turnaround  Waiting  Energy
1    6.00   0.00     8.00       8.00        2.00     15.20
2    4.00   1.00     5.00       4.00        0.00     10.80
3    8.00   2.00     16.00      14.00       6.00     20.40

- **PID**: Process identifier
- **Burst**: Total execution time needed by the process
- **Arrival**: Time when process enters the system
- **Completion**: Time when process finishes
- **Turnaround**: Total time in system (Completion time - Arrival time)
- **Waiting**: Time spent waiting (Turnaround - Burst)
- **Energy**: Energy units consumed

### Overall Statistics

Total Energy Consumption: 46.40 units
Average Turnaround Time: 8.67
Average Waiting Time: 2.67
Total Execution Time: 16.00


## Technical Details

### Process States
- **NEW**: Process is created
- **READY**: Process is waiting to be executed
- **RUNNING**: Process is currently executing
- **WAITING**: Process is waiting for I/O 
- **TERMINATED**: Process has completed execution

### Frequency Scaling Formula

frequency = minFreq + (maxFreq - minFreq) * (
    0.4 * (1 - completion_ratio) +
    0.3 * workload_factor +
    0.3 * priority_factor
)

### Energy Calculation
- Energy consumption is proportional to f³ * t
- f = CPU frequency
- t = execution time


