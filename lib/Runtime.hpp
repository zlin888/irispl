#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "Process.hpp"
#include "ModuleLoader.hpp"
#include "schemeObject.hpp"

#include <string>
#include <map>
#include <queue>
#include <stdexcept>

using namespace std;

class Runtime {
public:
    map<PID, Process> processPool;
    queue<Process> processQueue;
    std::shared_ptr<Process> currentProcessPtr;

    void schedule();

    void execute();

    int addProcess(Process process);

    Process createProcess(ModuleLoader moduleLoader);

    PID allocatePID();

    void ailStore();

    void ailLoad();

    void ailCall();
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
        this->currentProcessPtr = static_cast<shared_ptr<Process>>(&this->processQueue.front());
        this->processQueue.pop();

        // Run the current process
        // Round Robin Scheduling
        // Set time slice as 20 (execute 20 instructions each time), then switch to next process
        this->currentProcessPtr->state = RUNNING;
        for (int timeSlice = 0; timeSlice < 20; ++timeSlice) {
            this->execute();
            switch (this->currentProcessPtr->state) {
                case RUNNING:
                    continue;
                default:
                    break;
            }
        }

        // put the unfinished process to the back of the queue
        if (this->currentProcessPtr->state == RUNNING) {
            this->currentProcessPtr->state = READY;
            this->processQueue.push(*this->currentProcessPtr);
        }
    }
}

void Runtime::execute() {

    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.type != InstructionType::COMMENT && instruction.type != InstructionType::LABEL) {
        string argument = instruction.argument;
        string mnemonic = instruction.mnemonic;

        if (mnemonic == "store") {
            this->ailStore();
        } else if (mnemonic == "load") {
            this->ailLoad();
        }
    } else {
        this->currentProcessPtr->step();
    }

    if (this->currentProcessPtr->PC >= this->currentProcessPtr->instructions.size()) {
        this->currentProcessPtr->state = STOPPED;
    }
}


//=================================================================
//              Basic Instruction : Load and Store
//=================================================================

void Runtime::ailStore() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.argumentType != "VARIABLE")
        throw std::invalid_argument("[ERROR] store argument is not a variable -- aliStore");

    string variableName = instruction.argument;
    string variableValue = this->currentProcessPtr->popOperand();

    this->currentProcessPtr->currentClosurePtr->setBoundVariable(variableName, variableValue, true);
    this->currentProcessPtr->step();
}

void Runtime::ailLoad() {
    // Unfinished
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.argumentType != "VARIABLE")
        throw std::invalid_argument("[ERROR] store argument is not a variable -- aliStore");

    string variableName = instruction.argument;
    string variableValue = this->currentProcessPtr->dereference(variableName);

    this->currentProcessPtr->step();
}


//=================================================================
//                     Jump Instruction
//=================================================================

void Runtime::ailCall() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    this->currentProcessPtr->pushStackFrame(this->currentProcessPtr->currentClosurePtr, this->currentProcessPtr->PC + 1)

}

#endif // !RUNTIME_HPP