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
    map<PID, std::shared_ptr<Process>> processPool;
    queue<std::shared_ptr<Process>> processQueue;
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
    auto processPtr = std::shared_ptr<Process>(new Process(process));
    if (!processPool.count(process.pid)) {
        //process not in pool
        processPool.emplace(process.pid, processPtr);
    }
    processQueue.push(processPtr);
    return process.pid;
} ;

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
        this->currentProcessPtr = this->processQueue.front();
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
            this->processQueue.push(this->currentProcessPtr);
        }
    }
}

void Runtime::execute() {

    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.type != InstructionType::COMMENT && instruction.type != InstructionType::LABEL) {
        string argument = instruction.argument;
        string mnemonic = instruction.mnemonic;

        try {
            if (mnemonic == "store") {
                this->ailStore();
            } else if (mnemonic == "load") {
                this->ailLoad();
            } else if (mnemonic == "call") {
                this->ailCall();
            } else {
                this->currentProcessPtr->step();
            }
        } catch (exception &e) {
            std::cout << "-> Runtime::execute, when running (" << instruction.instructionStr << ")\n";
            std::cout << "--> " << e.what() << "\n";
            exit(0);
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
    if (instruction.argumentType != ArgumentType::VARIABEL)
        throw std::invalid_argument("[ERROR] store argument is not a variable : aliStore");

    string variableName = instruction.argument;
    string variableValue = this->currentProcessPtr->popOperand();

    this->currentProcessPtr->currentClosurePtr->setBoundVariable(variableName, variableValue, true);
    this->currentProcessPtr->step();
}

void Runtime::ailLoad() {
    // Unfinished
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.argumentType != ArgumentType::VARIABEL)
        throw std::invalid_argument("[ERROR] load argument is not a variable : aliLoad");

    string variableName = instruction.argument;
    string variableValue = this->currentProcessPtr->dereference(variableName);

    this->currentProcessPtr->step();
}


//=================================================================
//                     Jump Instruction
//=================================================================

void Runtime::ailCall() {
    // TODO : argument type is not label
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    this->currentProcessPtr->pushCurrentClosure(this->currentProcessPtr->PC + 1);

    if (instruction.argumentType == ArgumentType::LABEL) {
        //  arg[0] == '@'
        int callInstructionAddress = this->currentProcessPtr->labelLineMap[instruction.argument];
        string label = instruction.argument;

        Handle newClosureHandle = this->currentProcessPtr->newClosure(callInstructionAddress);
//        this->currentProcessPtr->currentClosurePtr->setFreeVariable("jack", "12", false);
//        this->currentProcessPtr->currentClosurePtr->setBoundVariable("ijack", "88", false);

        auto freeVariables = this->currentProcessPtr->currentClosurePtr->freeVariables;
        for (auto & freeVariable : freeVariables) {
            this->currentProcessPtr->getClosure(newClosureHandle)->setFreeVariable(freeVariable.first, freeVariable.second, false);
        }

        auto boundVariables = this->currentProcessPtr->currentClosurePtr->boundVariables;
        for (auto & boundVariable : boundVariables) {
            this->currentProcessPtr->getClosure(newClosureHandle)->setBoundVariable(boundVariable.first, boundVariable.second, false);
        }

        this->currentProcessPtr->setCurrentClosure(newClosureHandle);
        int instructionAddress = this->currentProcessPtr->labelLineMap[label];
        this->currentProcessPtr->gotoAddress(instructionAddress);
    } else {
        this->currentProcessPtr->step();
    }

}

#endif // !RUNTIME_HPP