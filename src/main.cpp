#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <thread>
#include <map>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

// Process States
enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

// Process Class Definition
class Process {
private:
    int pid;
    double burstTime;
    double remainingTime;
    double arrivalTime;
    double energyConsumption;
    ProcessState state;
    int priority;
    double cpuFrequency;
    double completionTime;
    double waitingTime;
    double turnaroundTime;

public:
    Process(int pid, double burstTime, double arrivalTime, int priority = 0)
        : pid(pid),
          burstTime(burstTime),
          remainingTime(burstTime),
          arrivalTime(arrivalTime),
          energyConsumption(0.0),
          state(ProcessState::NEW),
          priority(priority),
          cpuFrequency(1.0),
          completionTime(0.0),
          waitingTime(0.0),
          turnaroundTime(0.0) {}

    // Getters
    int getPID() const { return pid; }
    double getBurstTime() const { return burstTime; }
    double getRemainingTime() const { return remainingTime; }
    double getArrivalTime() const { return arrivalTime; }
    double getEnergyConsumption() const { return energyConsumption; }
    ProcessState getState() const { return state; }
    int getPriority() const { return priority; }
    double getCPUFrequency() const { return cpuFrequency; }
    double getCompletionTime() const { return completionTime; }
    double getWaitingTime() const { return waitingTime; }
    double getTurnaroundTime() const { return turnaroundTime; }

    // Setters
    void setState(ProcessState newState) { state = newState; }
    void setRemainingTime(double time) { remainingTime = time; }
    void setCPUFrequency(double freq) { cpuFrequency = freq; }
    void setCompletionTime(double time) { 
        completionTime = time;
        turnaroundTime = completionTime - arrivalTime;
        waitingTime = turnaroundTime - burstTime;
    }

    void updateEnergyConsumption(double timeSlice) {
        // E ∝ f³ * t
        double powerFactor = pow(cpuFrequency, 3);
        energyConsumption += powerFactor * timeSlice;
        remainingTime -= timeSlice * cpuFrequency;
    }

    bool isCompleted() const { return remainingTime <= 0; }
};

class EnergyEfficientScheduler {
private:
    struct ProcessCompare {
        bool operator()(const shared_ptr<Process>& a, const shared_ptr<Process>& b) {
            if (a->getPriority() != b->getPriority())
                return a->getPriority() < b->getPriority();
            return a->getRemainingTime() > b->getRemainingTime();
        }
    };

    priority_queue<shared_ptr<Process>, 
                  vector<shared_ptr<Process>>, 
                  ProcessCompare> readyQueue;
    vector<shared_ptr<Process>> allProcesses;
    vector<shared_ptr<Process>> completedProcesses;
    vector<pair<double, double>> energyHistory;

    double currentTime;
    double totalEnergyConsumption;
    const double timeQuantum;
    const double maxFrequency;
    const double minFrequency;
    double avgWorkload;
    int processCount;

    void adjustFrequency(shared_ptr<Process> process) {
        double optimalFreq = calculateOptimalFrequency(process);
        process->setCPUFrequency(optimalFreq);
    }

    double calculateOptimalFrequency(shared_ptr<Process> process) {
        double completion_ratio = process->getRemainingTime() / process->getBurstTime();
        double workload_factor = avgWorkload / (processCount > 0 ? processCount : 1);
        double priority_factor = process->getPriority() / 10.0;

        double freq = minFrequency + 
                     (maxFrequency - minFrequency) * 
                     (0.4 * (1 - completion_ratio) + 
                      0.3 * workload_factor + 
                      0.3 * priority_factor);

        return max(minFrequency, min(freq, maxFrequency));
    }

    void updateWorkloadMetrics(shared_ptr<Process> process) {
        avgWorkload = (avgWorkload * processCount + process->getRemainingTime()) / (processCount + 1);
        processCount++;
    }

public:
    EnergyEfficientScheduler(double timeQuantum = 1.0, 
                            double maxFreq = 2.0, 
                            double minFreq = 0.5)
        : currentTime(0.0),
          totalEnergyConsumption(0.0),
          timeQuantum(timeQuantum),
          maxFrequency(maxFreq),
          minFrequency(minFreq),
          avgWorkload(0.0),
          processCount(0) {}

    void addProcess(shared_ptr<Process> process) {
        allProcesses.push_back(process);
        updateWorkloadMetrics(process);
    }

    void run() {
        sort(allProcesses.begin(), allProcesses.end(),
             [](const auto& a, const auto& b) {
                 return a->getArrivalTime() < b->getArrivalTime();
             });

        size_t nextProcess = 0;

        while (nextProcess < allProcesses.size() || !readyQueue.empty()) {
            while (nextProcess < allProcesses.size() && 
                   allProcesses[nextProcess]->getArrivalTime() <= currentTime) {
                allProcesses[nextProcess]->setState(ProcessState::READY);
                readyQueue.push(allProcesses[nextProcess]);
                nextProcess++;
            }

            if (readyQueue.empty()) {
                if (nextProcess < allProcesses.size()) {
                    currentTime = allProcesses[nextProcess]->getArrivalTime();
                }
                continue;
            }

            auto currentProcess = readyQueue.top();
            readyQueue.pop();

            currentProcess->setState(ProcessState::RUNNING);
            adjustFrequency(currentProcess);

            double timeSlice = min(timeQuantum, 
                                 currentProcess->getRemainingTime() / 
                                 currentProcess->getCPUFrequency());
            
            double powerFactor = pow(currentProcess->getCPUFrequency(), 3);
            totalEnergyConsumption += powerFactor * timeSlice;
            currentProcess->updateEnergyConsumption(timeSlice);
            
            currentTime += timeSlice;
            energyHistory.push_back(make_pair(currentTime, totalEnergyConsumption));

            if (currentProcess->isCompleted()) {
                currentProcess->setState(ProcessState::TERMINATED);
                currentProcess->setCompletionTime(currentTime);
                completedProcesses.push_back(currentProcess);
            } else {
                currentProcess->setState(ProcessState::READY);
                readyQueue.push(currentProcess);
            }
        }
    }

    void printStatistics() const {
        cout << "\n=== Scheduling Statistics ===\n";
        cout << fixed << setprecision(2);
        
        cout << "\nProcess Statistics:\n";
        cout << "PID\tBurst\tArrival\tCompletion\tTurnaround\tWaiting\tEnergy\n";
        
        double totalTurnaround = 0;
        double totalWaiting = 0;
        
        for (const auto& process : completedProcesses) {
            cout << process->getPID() << "\t"
                 << process->getBurstTime() << "\t"
                 << process->getArrivalTime() << "\t"
                 << process->getCompletionTime() << "\t\t"
                 << process->getTurnaroundTime() << "\t\t"
                 << process->getWaitingTime() << "\t"
                 << process->getEnergyConsumption() << "\n";
                     
            totalTurnaround += process->getTurnaroundTime();
            totalWaiting += process->getWaitingTime();
        }

        size_t numProcesses = completedProcesses.size();
        cout << "\nOverall Statistics:\n";
        cout << "Total Energy Consumption: " << totalEnergyConsumption << " units\n";
        cout << "Average Turnaround Time: " << (numProcesses > 0 ? totalTurnaround / numProcesses : 0) << "\n";
        cout << "Average Waiting Time: " << (numProcesses > 0 ? totalWaiting / numProcesses : 0) << "\n";
        cout << "Total Execution Time: " << currentTime << "\n";
    }

    void printEnergyHistory() const {
        cout << "\nEnergy Consumption History:\n";
        cout << "Time\tEnergy Consumption\n";
        for (const auto& entry : energyHistory) {
            cout << entry.first << "\t" << entry.second << "\n";
        }
    }

    double getTotalEnergyConsumption() const { return totalEnergyConsumption; }
    double getCurrentTime() const { return currentTime; }
    const vector<shared_ptr<Process>>& getCompletedProcesses() const { 
        return completedProcesses; 
    }
};

int main() {
    cout << "\n=== Energy-Efficient CPU Scheduler ===\n";
    
    double timeQuantum;
    cout << "\nEnter time quantum for the scheduler (in ms): ";
    cin >> timeQuantum;
    
    while (timeQuantum <= 0) {
        cout << "Time quantum must be positive. Enter again: ";
        cin >> timeQuantum;
    }
    
    EnergyEfficientScheduler scheduler(timeQuantum);

    int n;
    cout << "\nEnter the number of processes: ";
    cin >> n;

    while (n <= 0) {
        cout << "Please enter a valid number of processes (greater than 0): ";
        cin >> n;
    }

    for (int i = 0; i < n; i++) {
        double burstTime, arrivalTime;
        int priority;

        cout << "\nProcess " << (i + 1) << " details:\n";
        
        cout << "Enter burst time (ms): ";
        cin >> burstTime;
        while (burstTime <= 0) {
            cout << "Burst time must be positive. Enter again: ";
            cin >> burstTime;
        }

        cout << "Enter arrival time (ms): ";
        cin >> arrivalTime;
        while (arrivalTime < 0) {
            cout << "Arrival time cannot be negative. Enter again: ";
            cin >> arrivalTime;
        }

        cout << "Enter priority (1-10, 1 being highest): ";
        cin >> priority;
        while (priority < 1 || priority > 10) {
            cout << "Priority must be between 1 and 10. Enter again: ";
            cin >> priority;
        }

        auto process = make_shared<Process>(i + 1, burstTime, arrivalTime, priority);
        scheduler.addProcess(process);
    }

    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    cout << "\nStarting CPU scheduling simulation...\n";
    cout << "Processing...\n";
    
    #ifdef _WIN32
        Sleep(500);
    #else
        usleep(500000);
    #endif
    
    scheduler.run();
    scheduler.printStatistics();

    char choice;
    do {
        cout << "\n=== Additional Options ===\n";
        cout << "1. View Energy Consumption History\n";
        cout << "2. Exit\n";
        cout << "Enter your choice (1-2): ";
        cin >> choice;

        switch (choice) {
            case '1':
                scheduler.printEnergyHistory();
                break;
            case '2':
                cout << "\nExiting program...\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != '2');

    return 0;
}