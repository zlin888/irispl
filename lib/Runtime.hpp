#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "Process.hpp"
#include "ModuleLoader.hpp"
#include "SchemeObject.hpp"

#include <string>
#include <map>
#include <queue>
#include <stdexcept>
#include <cmath>

using namespace std;

class Runtime {
public:
    map<PID, std::shared_ptr<Process>> processPool;
    queue<std::shared_ptr<Process>> processQueue;
    std::shared_ptr<Process> currentProcessPtr;

    void schedule();

    void execute();

    int addProcess(Process process);

    PID allocatePID();

    void ailStore();

    void ailLoad();

    void ailPush();

    void ailReturn();

    void ailHalt();

    void ailCall(const Instruction &instruction);

    void ailAdd();

    void ailDisplay();

    void aliLoadClosure();

    void ailCall(const Instruction &instruction, bool isTailCall);

    void ailTailCall(const Instruction &instruction);

    Process createProcess(Module module);

    void aliDisplay();

    void ailSub();

    void ailDiv();

    void ailMul();

    void ailMod();

    void ailPow();

    void ailEqn();

    void ailGe();

    void ailLe();

    void ailGt();

    void ailLt();

    void ailNot();

    void ailAnd();

    void ailOr();

    void ailIseq();

    void ailIsnull();

    void ailIsatom();

    void ailIslist();

    void ailIsnumber();
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
Process Runtime::createProcess(Module module) {
    Process process(this->allocatePID(), module);
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

//        try {
        if (mnemonic == "store") { this->ailStore(); }
        else if (mnemonic == "load") { this->ailLoad(); }
        else if (mnemonic == "call") { this->ailCall(instruction); }
        else if (mnemonic == "tailcall") { this->ailTailCall(instruction); }
        else if (mnemonic == "push") { this->ailPush(); }
        else if (mnemonic == "return") { this->ailReturn(); }
        else if (mnemonic == "halt") { this->ailHalt(); }

        else if (mnemonic == "add") { this->ailAdd(); }
        else if (mnemonic == "sub") { this->ailSub(); }
        else if (mnemonic == "mul") { this->ailMul(); }
        else if (mnemonic == "div") { this->ailDiv(); }
        else if (mnemonic == "mod") { this->ailMod(); }
        else if (mnemonic == "pow") { this->ailPow(); }
        else if (mnemonic == "eqn") { this->ailEqn(); }
        else if (mnemonic == "ge") { this->ailGe(); }
        else if (mnemonic == "le") { this->ailLe(); }
        else if (mnemonic == "gt") { this->ailGt(); }
        else if (mnemonic == "lt") { this->ailLt(); }
        else if (mnemonic == "not") { this->ailNot(); }
        else if (mnemonic == "and") { this->ailAnd(); }
        else if (mnemonic == "or") { this->ailOr(); }
        else if (mnemonic == "eq?") { this->ailIseq(); }
        else if (mnemonic == "null?") { this->ailIsnull(); }
        else if (mnemonic == "atom?") { this->ailIsatom(); }
        else if (mnemonic == "list?") { this->ailIslist(); }
        else if (mnemonic == "number?") { this->ailIsnumber(); }


        else if (mnemonic == "display") { this->ailDisplay(); }
        else {
            this->currentProcessPtr->step();
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
    if (instruction.argumentType != InstructionArgumentType::VARIABLE)
        throw std::invalid_argument("[ERROR] store argument is not a variable : aliStore");

    string variableName = instruction.argument;
    string variableValue = this->currentProcessPtr->popOperand();

    this->currentProcessPtr->currentClosurePtr->setBoundVariable(variableName, variableValue, false);
    this->currentProcessPtr->step();
}

void Runtime::ailLoad() {
    // Unfinished
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.argumentType == InstructionArgumentType::VARIABLE) {
        string variableName = instruction.argument;
        string variableValue = this->currentProcessPtr->dereference(variableName);

        Instruction tempInstruction("load " + variableValue);

        if (tempInstruction.argumentType == InstructionArgumentType::LABEL) {
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

void Runtime::aliLoadClosure() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.argumentType == InstructionArgumentType::LABEL) {
        string label = instruction.argument;

    } else {
        throw std::invalid_argument("[ERROR] loadclosure argument is not a label: aliLoadClosure");
    }
//    let argType = TypeOfToken(argument);
//
//    if(argType !== 'LABEL') { throw `[Error] loadclosure指令参数类型不是标签`; }

//    let label = argument;
//    let instAddress = PROCESS.GetLabelAddress(label);
//    let newClosureHandle = PROCESS.NewClosure(instAddress, PROCESS.currentClosureHandle);
//    let currentClosure = PROCESS.GetCurrentClosure();
//    for(let v in currentClosure.freeVariables) {
//        let value = currentClosure.GetFreeVariable(v);
//        PROCESS.GetClosure(newClosureHandle).InitFreeVariable(v, value);
//    }
//    for(let v in currentClosure.boundVariables) {
//        let value = currentClosure.GetBoundVariable(v);
//        PROCESS.GetClosure(newClosureHandle).InitFreeVariable(v, value);
//    }
//    PROCESS.PushOperand(newClosureHandle);
//    PROCESS.Step();
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
    this->ailCall(instruction, false);
}

void Runtime::ailCall(const Instruction &instruction, bool isTailCall) {
    // Push the current closure to the fstack for storage, it will be reused after "return" of the new function

    //  arg[0] == '@' : LABEL
    if (instruction.argumentType == InstructionArgumentType::LABEL) {

        if (!isTailCall) {
            this->currentProcessPtr->pushStackFrame(this->currentProcessPtr->currentClosurePtr,
                                                    this->currentProcessPtr->PC + 1);
        }

        int callInstructionAddress = this->currentProcessPtr->labelAddressMap[instruction.argument];
        string label = instruction.argument;

        // create a new closure for the function execution
        Handle newClosureHandle = this->currentProcessPtr->newClosure(callInstructionAddress);

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
        int instructionAddress = this->currentProcessPtr->labelAddressMap[label];
        this->currentProcessPtr->gotoAddress(instructionAddress);

    } else if (instruction.argumentType == InstructionArgumentType::HANDLE) {

        if (!isTailCall) {
            this->currentProcessPtr->pushStackFrame(this->currentProcessPtr->currentClosurePtr,
                                                    this->currentProcessPtr->PC + 1);
        }

        Handle handle = instruction.argument;
        shared_ptr<SchemeObject> schemeObjPtr = this->currentProcessPtr->heap.get(handle);

        if (schemeObjPtr->schemeObjectType == SchemeObjectType::CLOSURE) {
            auto closurePtr = static_pointer_cast<Closure>(schemeObjPtr);
            this->currentProcessPtr->setCurrentClosure(handle);
            this->currentProcessPtr->gotoAddress(closurePtr->instructionAddress);
        }

    } else if (instruction.argumentType == InstructionArgumentType::VARIABLE) {
        // TODO native calls
        string variableName = instruction.argument;
        string variableValue = this->currentProcessPtr->dereference(variableName);

        Instruction newInstruction("call " + variableValue);
        this->ailCall(newInstruction);
    } else {
        throw std::runtime_error("[ERROR] call's argument must be handle, label, or variable : Runtime::ailCall");
    }
}

void Runtime::ailTailCall(const Instruction &instruction) {
    this->ailCall(instruction, true);

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

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        // TODO: NUMBER is not enough, we need better integer system here
        this->currentProcessPtr->pushOperand(to_string(stod(operand1) + stod(operand2)));
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailSub() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        this->currentProcessPtr->pushOperand(to_string(stod(operand1) - stod(operand2)));
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailDiv() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        this->currentProcessPtr->pushOperand(to_string(stod(operand1) / stod(operand2)));
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailMul() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        this->currentProcessPtr->pushOperand(to_string(stod(operand1) * stod(operand2)));
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailMod() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        this->currentProcessPtr->pushOperand(to_string((int) (stod(operand1) + 0.5) % (int) (stod(operand2) + 0.5)));
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailPow() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        this->currentProcessPtr->pushOperand(to_string(pow(stod(operand1), stod(operand2))));
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailEqn() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        this->currentProcessPtr->pushOperand(
                std::fabs(stod(operand1) - stod(operand2)) <= std::numeric_limits<double>::epsilon() ? "#t" : "#f");
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();
}

// >=
void Runtime::ailGe() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        this->currentProcessPtr->pushOperand(
                stod(operand1) >= stod(operand2) ? "#t" : "#f");
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();

}

void Runtime::ailLe() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        this->currentProcessPtr->pushOperand(
                stod(operand1) <= stod(operand2) ? "#t" : "#f");
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();

}

void Runtime::ailGt() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        this->currentProcessPtr->pushOperand(
                stod(operand1) > stod(operand2) ? "#t" : "#f");
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();

}

void Runtime::ailLt() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        this->currentProcessPtr->pushOperand(
                stod(operand1) < stod(operand2) ? "#t" : "#f");
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailNot() {
    string operand = this->currentProcessPtr->popOperand();
    this->currentProcessPtr->pushOperand(operand == "#f" ? "#t" : "#f");
    this->currentProcessPtr->step();
}

void Runtime::ailAnd() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if(operand1 == "#f" || operand2 == "#f") {
        this->currentProcessPtr->pushOperand("#f");
    } else {
        this->currentProcessPtr->pushOperand("#t");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailOr() {
    string operand1 = this->currentProcessPtr->popOperand();
    string operand2 = this->currentProcessPtr->popOperand();

    if(operand1 == "#t" || operand2 == "#t") {
        this->currentProcessPtr->pushOperand("#t");
    } else {
        this->currentProcessPtr->pushOperand("#f");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailIseq() {

}

void Runtime::ailIsnull() {

}

void Runtime::ailIsatom() {

}

void Runtime::ailIslist() {

}

void Runtime::ailIsnumber() {
    string operand = this->currentProcessPtr->popOperand();
    this->currentProcessPtr->pushOperand(typeOfStr(operand) == Type::NUMBER ? "#t" : "#f");
    this->currentProcessPtr->step();

}

//=================================================================
//                      Other Instructions
//=================================================================

void Runtime::ailDisplay() {
    string argument = this->currentProcessPtr->popOperand();
    Type argumentType = typeOfStr(argument);

    if (argumentType == Type::HANDLE) {
        shared_ptr<SchemeObject> schemeObjectPtr = this->currentProcessPtr->heap.get(argument);
        if (schemeObjectPtr->schemeObjectType == SchemeObjectType::STRING) {
            auto stringObjPtr = static_pointer_cast<StringObject>(schemeObjectPtr);
            cout << stringObjPtr->content << endl;
        }
    } else if (argumentType == Type::NUMBER || argumentType == Type::BOOLEAN) {
        cout << argument << endl;
    }

    this->currentProcessPtr->step();
}


#endif // !RUNTIME_HPP