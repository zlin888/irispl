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

    void ailPush();

    void ailReturn();

    void ailHalt();

    void ailCall(const Instruction &instruction);

    void ailAdd();

    void ailDisplay();
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
        this->currentProcessPtr = this->processQueue.front();
        this->processQueue.pop();

        // Run the current process
        // Round Robin Scheduling
        // Set time slice as 20 (execute 20 instructions each time), then switch to next process
        this->currentProcessPtr->state = ProcessState::RUNNING;
        for (int timeSlice = 0; timeSlice < 20; ++timeSlice) {
            this->execute();
            switch (this->currentProcessPtr->state) {
                case ProcessState::RUNNING:
                    continue;
                default:
                    break;
            }
        }

        // put the unfinished process to the back of the queue
        if (this->currentProcessPtr->state == ProcessState::RUNNING) {
            this->currentProcessPtr->state = ProcessState::READY;
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
                this->ailCall(instruction);
            } else if (mnemonic == "push") {
                this->ailPush();
            } else if (mnemonic == "return") {
                this->ailReturn();
            } else if (mnemonic == "halt") {
                this->ailHalt();
            } else if (mnemonic == "add") {
                ailAdd();
            }
            else {
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
        this->currentProcessPtr->state = ProcessState::STOPPED;
    }
}


//=================================================================
//              Basic Instruction : Load and Store
//=================================================================

void Runtime::ailStore() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.argumentType != ArgumentType::VARIABLE)
        throw std::invalid_argument("[ERROR] store argument is not a variable : aliStore");

    string variableName = instruction.argument;
    string variableValue = this->currentProcessPtr->popOperand();

    this->currentProcessPtr->currentClosurePtr->setBoundVariable(variableName, variableValue, false);
    this->currentProcessPtr->step();
}

void Runtime::ailLoad() {
    // Unfinished
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.argumentType == ArgumentType::VARIABLE) {
        string variableName = instruction.argument;
        string variableValue = this->currentProcessPtr->dereference(variableName);

        Instruction tempInstruction("load " + variableValue);

        if (tempInstruction.argumentType == ArgumentType::LABEL) {
            // TODO loadclosure
            this->currentProcessPtr->step();
        } else {
            this->currentProcessPtr->pushOperand(tempInstruction.argument);
            this->currentProcessPtr->step();
        }
    } else {
        throw std::invalid_argument("[ERROR] load argument is not a variable : aliLoad");
    }
}

void Runtime::ailPush() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    this->currentProcessPtr->pushOperand(instruction.argument);
    this->currentProcessPtr->step();
}

//=================================================================
//                     Jump Instruction
//=================================================================

void Runtime::ailCall(const Instruction &instruction) {
    // TODO : argument type is not label neither variable;

    // Push the current closure to the fstack for storage, it will be reused after "return" of the new function

    //  arg[0] == '@' : LABEL
    if (instruction.argumentType == ArgumentType::LABEL) {


        this->currentProcessPtr->pushStackFrame(this->currentProcessPtr->currentClosurePtr,
                                                this->currentProcessPtr->PC + 1);

        int callInstructionAddress = this->currentProcessPtr->labelLineMap[instruction.argument];
        string label = instruction.argument;

        // create a new closure for the function execution
        Handle newClosureHandle = this->currentProcessPtr->newClosure(callInstructionAddress);
//        this->currentProcessPtr->currentClosurePtr->setFreeVariable("jack", "12", false);
//        this->currentProcessPtr->currentClosurePtr->setBoundVariable("ijack", "88", false);

        // Copy the bound and free variables from the current closure to the new closure
        auto freeVariables = this->currentProcessPtr->currentClosurePtr->freeVariables;
        for (auto &freeVariable : freeVariables) {
            this->currentProcessPtr->getClosurePtr(newClosureHandle)->setFreeVariable(freeVariable.first,
                                                                                      freeVariable.second, false);
        }

        auto boundVariables = this->currentProcessPtr->currentClosurePtr->boundVariables;
        for (auto &boundVariable : boundVariables) {
            this->currentProcessPtr->getClosurePtr(newClosureHandle)->setBoundVariable(boundVariable.first,
                                                                                       boundVariable.second, false);
        }

        // Set the current closure to the new closure and then head to the new function's instructions
        this->currentProcessPtr->setCurrentClosure(newClosureHandle);
        int instructionAddress = this->currentProcessPtr->labelLineMap[label];
        this->currentProcessPtr->gotoAddress(instructionAddress);

    } else if (instruction.argumentType == ArgumentType::VARIABLE) {
        // TODO native calls
        string variableName = instruction.argument;
        string variableValue = this->currentProcessPtr->dereference(variableName);

        Instruction newInstruction("call " + variableValue);
        this->ailCall(newInstruction);
    } else {
        throw std::runtime_error("[ERROR] call's argument must be handle, label, or variable : Runtime::ailCall");
    }
}

void Runtime::ailReturn() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    StackFrame sf = this->currentProcessPtr->popStackFrame();
    this->currentProcessPtr->currentClosurePtr = sf.closurePtr;
    this->currentProcessPtr->gotoAddress(sf.returnAddress);
}


void Runtime::ailHalt() {
    this->currentProcessPtr->state = ProcessState::STOPPED;
}


//=================================================================
//                      Calculation
//=================================================================

void Runtime::ailAdd() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == ArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == ArgumentType::NUMBER) {
        // TODO: NUMBER is not enough, we need better integer system here
        this->currentProcessPtr->pushOperand(to_string(stod(operand1) + stod(operand2)));
    }

    this->currentProcessPtr->step();
}


//=================================================================
//                      Other Instructions
//=================================================================

void Runtime::ailDisplay() {
    string argument = this->currentProcessPtr->popOperand();
    ArgumentType argumentType = Instruction::getArgumentType(argument);

    if (argumentType == ArgumentType::HANDLE) {
        shared_ptr<SchemeObject> schemeObjectPtr = this->currentProcessPtr->heap.get(argument);
        if (schemeObjectPtr->schemeObjectType == SchemeObjectType::STRING) {
            //TODO display string
        }
    }
}
#endif // !RUNTIME_HPP