#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "Process.hpp"
#include "ModuleLoader.hpp"

#include <string>
#include <map>
#include <queue>
#include <stdexcept>

using namespace std;

class Runtime {
public:
    map<PID, Process> processPool;
    queue<Process> processQueue;
    Process currentProcess;

    void schedule();

    void execute();

    int addProcess(Process process);

    Process createProcess(ModuleLoader moduleLoader);

    PID allocatePID();

    void ailStore();

    void ailLoad();
};


//=================================================================
//                      PROCESS RELATED
//=================================================================

int Runtime::addProcess(Process process) {
    if (!processPool.count(process.pid)) {
        //process not in pool 
        processPool.emplace(process.pid, process);
    }
    processQueue.push(process);
    return process.pid;
};

// process factory, allocate PID and help to pass module loader to the constructor of the process
Process Runtime::createProcess(ModuleLoader moduleLoader) {
    Process process(this->allocatePID(), moduleLoader);
    return process;
}

PID Runtime::allocatePID() {
    return processPool.size();
}


//=================================================================
//                      Scheduler
//=================================================================
void Runtime::schedule() {
    while (!this->processQueue.empty()) {
        // Pop from process queue
        this->currentProcess = this->processQueue.front();
        this->processQueue.pop();

        // Run the current process
        // Round Robin Scheduling
        // Set time slice as 20 (execute 20 instructions each time), then switch to next process
        this->currentProcess.state = RUNNING;
        for (int timeSlice = 0; timeSlice < 20; ++timeSlice) {
            this->execute();
            switch (this->currentProcess.state) {
                case RUNNING:
                    continue;
                default:
                    break;
            }
        }

        // put the unfinished process to the back of the queue
        if (this->currentProcess.state == RUNNING) {
            this->currentProcess.state = READY;
            this->processQueue.push(this->currentProcess);
        }
    }
}

void Runtime::execute() {

    Instruction instruction = this->currentProcess.currentInstruction();
    if (instruction.type != InstructionType::COMMENT && instruction.type != InstructionType::LABEL) {
        string argument = instruction.argument;
        string mnemonic = instruction.mnemonic;

        if (mnemonic == "store") {
            this->ailStore();
        } else if (mnemonic == "load") {
            this->ailStore();
        }
    }
    this->currentProcess.step();

    if (this->currentProcess.PC >= this->currentProcess.instructions.size()) {
        this->currentProcess.state = STOPPED;
    }
}


//=================================================================
//              Basic Instruction : Load and Store
//=================================================================

void Runtime::ailStore() {
    Instruction instruction = this->currentProcess.currentInstruction();
    if (instruction.argumentType != "VARIABLE")
        throw std::invalid_argument("[ERROR] store argument is not a variable -- aliStore");

    string variableName = instruction.argument;
    string variableValue = this->currentProcess.popOperand();

//    PROCESS.GetCurrentClosure().InitBoundVariable(variable, value);
//    PROCESS.Step();
}

void Runtime::ailLoad() {
    Instruction instruction = this->currentProcess.currentInstruction();
    if (instruction.argumentType != "VARIABLE")
        throw std::invalid_argument("[ERROR] store argument is not a variable -- aliStore");

    string variableName = instruction.argument;
    string variableValue = instruction.argument;

}
#endif // !RUNTIME_HPP