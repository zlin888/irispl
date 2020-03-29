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

string RUNTIME_PREFIX = "_!!!runtime_prefix!!!_";

enum class OutputMode {
    BUFFERED, UNBUFFERED
};

class Runtime {
public:
    map<PID, std::shared_ptr<Process>> processPool;
    queue<std::shared_ptr<Process>> processQueue;
    std::shared_ptr<Process> currentProcessPtr;
    vector<string> outputBuffer;
    OutputMode outputMode;

    inline Runtime() { this->outputMode = OutputMode::UNBUFFERED; };

    inline Runtime(OutputMode outputMode) : outputMode(outputMode) {};

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

    void ailIsEq();

    void ailIsnull();

    void ailIsatom();

    void ailIsList();

    void ailIsnumber();

    void ailFork();

    void ailNewline();

    void ailRead();

    void ailWrite();

    void ailNop();

    void ailPause();

    void ailSetchild();

    void ailConcat();

    void ailDuplicate();


    void ailPop();

    Handle newClosureBaseOnCurrentClosure(int instAddress);

    void ailSet();

    void output(string outputStr, bool is_with_endl);

    void output(string outputStr);

    void ailCar();

    void ailCdr();

    void ailCons();

    void ailIfTrue();

    void ailIfFalse();

    void ailGoto();

    vector<HandleOrStr> popOperandsInversely(int num);

    string toStr(HandleOrStr hos);

    void ailBegin();
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
        else if (mnemonic == "loadclosure") { this->aliLoadClosure(); }
        else if (mnemonic == "push") { this->ailPush(); }
        else if (mnemonic == "pop") { this->ailPop(); }
        else if (mnemonic == "set") { this->ailSet(); }


        else if (mnemonic == "call") { this->ailCall(instruction); }
        else if (mnemonic == "tailcall") { this->ailTailCall(instruction); }
        else if (mnemonic == "return") { this->ailReturn(); }
        else if (mnemonic == "iftrue") { this->ailIfTrue(); }
        else if (mnemonic == "iffalse") { this->ailIfFalse(); }
        else if (mnemonic == "goto") { this->ailGoto(); }

        else if (mnemonic == "car") { this->ailCar(); }
        else if (mnemonic == "cdr") { this->ailCdr(); }
        else if (mnemonic == "cons") { this->ailCons(); }

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
        else if (mnemonic == "eq?") { this->ailIsEq(); }
        else if (mnemonic == "null?") { this->ailIsnull(); }
        else if (mnemonic == "atom?") { this->ailIsatom(); }
        else if (mnemonic == "list?") { this->ailIsList(); }
        else if (mnemonic == "number?") { this->ailIsnumber(); }


        else if (mnemonic == "fork") { this->ailFork(); }
        else if (mnemonic == "display") { this->ailDisplay(); }
        else if (mnemonic == "newline") { this->ailNewline(); }
        else if (mnemonic == "read") { this->ailRead(); }
        else if (mnemonic == "write") { this->ailWrite(); }
        else if (mnemonic == "nop") { this->ailNop(); }
        else if (mnemonic == "pause") { this->ailPause(); }
        else if (mnemonic == "halt") { this->ailHalt(); }
        else if (mnemonic == "begin") { this->ailBegin(); }

        else if (mnemonic == "set-child!") { this->ailSetchild(); }
        else if (mnemonic == "concat") { this->ailConcat(); }
        else if (mnemonic == "duplicate") { this->ailDuplicate(); }
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

Handle Runtime::newClosureBaseOnCurrentClosure(int instAddress) {
    Handle newClosureHandle = this->currentProcessPtr->newClosure(instAddress);
    auto currentClosurePtr = this->currentProcessPtr->currentClosurePtr;

    auto freeVariables = this->currentProcessPtr->currentClosurePtr->freeVariables;
    for (auto &freeVariable : freeVariables) {
        this->currentProcessPtr->getClosurePtr(newClosureHandle)->setFreeVariable(freeVariable.first,
                                                                                  freeVariable.second,
                                                                                  false);
    }

    auto boundVariables = this->currentProcessPtr->currentClosurePtr->boundVariables;
    for (auto &boundVariable : boundVariables) {
        this->currentProcessPtr->getClosurePtr(newClosureHandle)->setBoundVariable(boundVariable.first,
                                                                                   boundVariable.second,
                                                                                   false);
    }
    return newClosureHandle;
}

// load variable, dereference and push the stask
void Runtime::ailLoad() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.argumentType == InstructionArgumentType::VARIABLE) {
        string argument = instruction.argument;
        string argumentValue = this->currentProcessPtr->dereference(argument);

        Type argumentValueType = typeOfStr(argumentValue);

        if (argumentValueType == Type::LABEL) {
            if (this->currentProcessPtr->labelAddressMap.count(argumentValue)) {
                int instAddress = this->currentProcessPtr->labelAddressMap[argumentValue];

                Handle newClosureHandle = this->newClosureBaseOnCurrentClosure(instAddress);

                this->currentProcessPtr->pushOperand(newClosureHandle);
                this->currentProcessPtr->step();
            } else {
                utils::log("label doesn't exist", __FILE__, __FUNCTION__, __LINE__);
                throw std::runtime_error("");
            }
        } else {
            // variable
            this->currentProcessPtr->pushOperand(argumentValue);
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

        if (this->currentProcessPtr->labelAddressMap.count(label)) {
            int instAddress = this->currentProcessPtr->labelAddressMap[label];

            Handle newClosureHandle = this->newClosureBaseOnCurrentClosure(instAddress);

            this->currentProcessPtr->pushOperand(newClosureHandle);
            this->currentProcessPtr->step();
        } else {
            utils::log("label doesn't exist", __FILE__, __FUNCTION__, __LINE__);
            throw std::runtime_error("");
        }

    } else {
        utils::log("loadclosure argument is not a label", __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

}

void Runtime::ailPush() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    this->currentProcessPtr->pushOperand(instruction.argument);
    this->currentProcessPtr->step();
}

void Runtime::ailPop() {
    this->currentProcessPtr->popOperand();
    this->currentProcessPtr->step();
}

void Runtime::ailSet() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    Type argumentType = instruction.argumentType;
    if (argumentType != Type::VARIABLE) {
        utils::log("argument is not a variable", __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    string variable = instruction.argument;
    string newValue = this->currentProcessPtr->popOperand();

    if (this->currentProcessPtr->currentClosurePtr->hasBoundVariable(variable)) {
        this->currentProcessPtr->currentClosurePtr->setBoundVariable(variable, newValue, true);
    }

    if (this->currentProcessPtr->currentClosurePtr->hasFreeVariable(variable)) {
        this->currentProcessPtr->currentClosurePtr->setFreeVariable(variable, newValue, true);
    }

    // track up, change the parent closure
    Handle currentClosureHandle = this->currentProcessPtr->currentClosurePtr->parentHandle;

    while (currentClosureHandle != TOP_NODE_HANDLE && this->currentProcessPtr->heap.hasHandle(currentClosureHandle)) {
        auto currentClosurePtr = this->currentProcessPtr->getClosurePtr(currentClosureHandle);
        if (currentClosurePtr->hasBoundVariable(variable)) {
            this->currentProcessPtr->currentClosurePtr->setBoundVariable(variable, newValue, true);
        }

        currentClosureHandle = currentClosurePtr->parentHandle;
    }

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
        Handle newClosureHandle = this->newClosureBaseOnCurrentClosure(callInstructionAddress);

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

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
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

    if (operand1 == "#f" || operand2 == "#f") {
        this->currentProcessPtr->pushOperand("#f");
    } else {
        this->currentProcessPtr->pushOperand("#t");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailOr() {
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

    if (operand1 == "#t" || operand2 == "#t") {
        this->currentProcessPtr->pushOperand("#t");
    } else {
        this->currentProcessPtr->pushOperand("#f");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailIsEq() {
    string operand2 = this->currentProcessPtr->popOperand();
    string operand1 = this->currentProcessPtr->popOperand();

    Type operand1Type = typeOfStr(operand1);
    Type operand2Type = typeOfStr(operand2);

    if (operand1Type != operand2Type) {
        this->currentProcessPtr->pushOperand("#f");
        this->currentProcessPtr->step();
        return;
    }

    if (operand1Type == Type::HANDLE) {
        auto schemeObjPtr1 = this->currentProcessPtr->heap.get(operand1);
        auto schemeObjPtr2 = this->currentProcessPtr->heap.get(operand2);

        if (schemeObjPtr1->schemeObjectType != schemeObjPtr2->schemeObjectType) {
            this->currentProcessPtr->pushOperand("#f");
            this->currentProcessPtr->step();
            return;
        }

        if (schemeObjPtr1->schemeObjectType == SchemeObjectType::QUOTE) {
            auto hoses1 = SchemeObject::getChildrenHosesOrBodies(schemeObjPtr1);
            auto hoses2 = SchemeObject::getChildrenHosesOrBodies(schemeObjPtr2);

            if (hoses1.size() != hoses2.size()) {
                this->currentProcessPtr->pushOperand("#f");
                this->currentProcessPtr->step();
                return;
            }

            for (int i = 0; i < hoses1.size(); ++i) {
                if (hoses1[i] != hoses2[i]) {
                    this->currentProcessPtr->pushOperand("#f");
                    this->currentProcessPtr->step();
                    return;
                }
            }

            this->currentProcessPtr->pushOperand("#t");
            this->currentProcessPtr->step();
            return;
        }

    }

    if (utils::makeSet<Type>(5, Type::BOOLEAN, Type::KEYWORD, Type::NUMBER, Type::STRING, Type::PORT).count(
            operand1Type)) {
        if (operand1 == operand2) {
            this->currentProcessPtr->pushOperand("#t");
        } else {
            this->currentProcessPtr->pushOperand("#f");
        }
    }

    this->currentProcessPtr->step();

}

void Runtime::ailIsnull() {

}

void Runtime::ailIsatom() {

}

void Runtime::ailIsList() {
    string argument = this->currentProcessPtr->popOperand();
    if (typeOfStr(argument) == Type::HANDLE) {
        auto schemeObjPtr = this->currentProcessPtr->heap.get(argument);
        if (schemeObjPtr->schemeObjectType != SchemeObjectType::LIST) {
            this->currentProcessPtr->pushOperand("#f");
        } else {
            this->currentProcessPtr->pushOperand("#t");
        }
    } else {
        this->currentProcessPtr->pushOperand("#f");
    }
    this->currentProcessPtr->step();
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
    this->output(this->toStr(argument), true);
    this->currentProcessPtr->step();
}

string Runtime::toStr(HandleOrStr hos) {
    Type hosType = typeOfStr(hos);
    if (hosType == Type::HANDLE) {
        shared_ptr<SchemeObject> schemeObjectPtr = this->currentProcessPtr->heap.get(hos);
        if (schemeObjectPtr->schemeObjectType == SchemeObjectType::STRING) {
            auto stringObjPtr = static_pointer_cast<StringObject>(schemeObjectPtr);
            return stringObjPtr->content;
        } else if (schemeObjectPtr->schemeObjectType == SchemeObjectType::QUOTE ||
                   schemeObjectPtr->schemeObjectType == SchemeObjectType::LIST) {
            string buffer = "(";
            auto hoses = SchemeObject::getChildrenHosesOrBodies(schemeObjectPtr);
            for (int i = 0; i < hoses.size(); ++i) {
                buffer += this->toStr(hoses[i]);
                if (i != hoses.size() - 1) {
                    buffer += " ";
                } else {
                    buffer += ")";
                }
            }
            return buffer;
        } else if (schemeObjectPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
            return "<lambda:" + hos + ">";
        }
    } else if (hosType == Type::NUMBER || hosType == Type::BOOLEAN || hosType == Type::KEYWORD) {
        return hos;
    }
    return hos;
}

void Runtime::output(string outputStr, bool is_with_endl) {
    if (this->outputMode == OutputMode::UNBUFFERED) {
        if (is_with_endl) {
            cout << outputStr << endl;
        } else {
            cout << outputStr;
        }
    } else if (this->outputMode == OutputMode::BUFFERED) {
        this->outputBuffer.push_back(outputStr);
    }
}

void Runtime::ailFork() {

}

void Runtime::ailNewline() {
    cout << "" << endl;
    this->currentProcessPtr->step();
}

void Runtime::ailRead() {

}

void Runtime::ailWrite() {

}

// blank instruction, do nothing
void Runtime::ailNop() {
    this->currentProcessPtr->step();
}

void Runtime::ailPause() {
    this->currentProcessPtr->state = ProcessState::SUSPENDED;
}

void Runtime::ailSetchild() {

}

void Runtime::ailConcat() {

}

void Runtime::ailDuplicate() {

}

void Runtime::ailCar() {
    HandleOrStr hos = this->currentProcessPtr->popOperand();
    Type hosType = typeOfStr(hos);

    if (hosType == Type::HANDLE) {
        shared_ptr<SchemeObject> schemeObjectPtr = this->currentProcessPtr->heap.get(hos);
        if (schemeObjectPtr->schemeObjectType == SchemeObjectType::LIST) {
            shared_ptr<ListObject> listObjStr = static_pointer_cast<ListObject>(schemeObjectPtr);
            this->currentProcessPtr->pushOperand(listObjStr->car());
        } else {
            throw std::invalid_argument(
                    "[ailCar] car's argument should be a List, but get a " + hos + " (" + TypeStrMap[hosType] + ") ");
        }

    } else {
        throw std::invalid_argument(
                "[ailCar] car's argument should be a List, but get a " + hos + " (" + TypeStrMap[hosType] + ") ");
    }
    this->currentProcessPtr->step();

}

void Runtime::ailCdr() {
    HandleOrStr hos = this->currentProcessPtr->popOperand();
    Type hosType = typeOfStr(hos);

    if (hosType == Type::HANDLE) {
        shared_ptr<SchemeObject> schemeObjectPtr = this->currentProcessPtr->heap.get(hos);
        if (schemeObjectPtr->schemeObjectType == SchemeObjectType::LIST) {
            shared_ptr<ListObject> listObjPtr = static_pointer_cast<ListObject>(schemeObjectPtr);

            if (listObjPtr->size() - 1 == 1) {
                // create a fake list point the real list (with currentIndex + 1)
                // * * * * * (list 1)
                //   * * * * (cdr list1) -> fake list 1
                Handle newListHandle = this->currentProcessPtr->heap.makeList(RUNTIME_PREFIX, TOP_NODE_HANDLE);
                auto newListObjPtr = static_pointer_cast<ListObject>(this->currentProcessPtr->heap.get(newListHandle));

                if (listObjPtr->isFake) {
                    newListObjPtr->pointTo(listObjPtr->realListObjPtr, listObjPtr->currentIndex + 1);
                } else {
                    newListObjPtr->pointTo(listObjPtr, 1);
                }

                this->currentProcessPtr->pushOperand(newListObjPtr->car());
            } else {
                Handle newListHandle = this->currentProcessPtr->heap.makeList(RUNTIME_PREFIX, TOP_NODE_HANDLE);
                auto newListObjPtr = static_pointer_cast<ListObject>(this->currentProcessPtr->heap.get(newListHandle));

                if (listObjPtr->isFake) {
                    newListObjPtr->pointTo(listObjPtr->realListObjPtr, listObjPtr->currentIndex + 1);
                } else {
                    newListObjPtr->pointTo(listObjPtr, 1);
                }

                this->currentProcessPtr->pushOperand(newListHandle);
            }

        } else {
            throw std::invalid_argument(
                    "[ailCdr] cdr's argument should be a List, but get a " + hos + " (" + TypeStrMap[hosType] + ") ");
        }

    } else {
        throw std::invalid_argument(
                "[ailCdr] cdr's argument should be a List, but get a " + hos + " (" + TypeStrMap[hosType] + ") ");
    }
    this->currentProcessPtr->step();
}

void Runtime::ailCons() {
    // create list, and push it
    // is actually push handle_to_list

    Handle handle = this->currentProcessPtr->heap.makeList(RUNTIME_PREFIX, TOP_NODE_HANDLE);
    shared_ptr<ListObject> listObjPtr = static_pointer_cast<ListObject>(this->currentProcessPtr->heap.get(handle));
    string argumentNum = this->currentProcessPtr->popOperand();

    vector<HandleOrStr> hoses = this->popOperandsInversely(stoi(argumentNum));
    for (auto hos : hoses) {
        listObjPtr->addChild(hos);
    }
    this->currentProcessPtr->pushOperand(handle);
    this->currentProcessPtr->step();
}

void Runtime::ailIfTrue() {
    string predicate = this->currentProcessPtr->popOperand();
    Type predicateType = typeOfStr(predicate);

    string argument = this->currentProcessPtr->currentInstruction().argument;
    Type argumentType = typeOfStr(argument);

    if (predicateType != Type::BOOLEAN) {
        throw std::invalid_argument("[ailIfTrue] predicate should be Boolean");
    }

    if (argumentType == Type::LABEL) {
        string label = argument;

        if (predicate == "#t") {
            int targetAddress = this->currentProcessPtr->labelAddressMap[label];
            this->currentProcessPtr->gotoAddress(targetAddress);
        } else {
            this->currentProcessPtr->step();
        }
    } else {
        throw std::invalid_argument("[ailIfTrue] argument should be Label");
    }
}

void Runtime::ailIfFalse() {
    string predicate = this->currentProcessPtr->popOperand();
    Type predicateType = typeOfStr(predicate);

    string argument = this->currentProcessPtr->currentInstruction().argument;
    Type argumentType = typeOfStr(argument);

    if (predicateType != Type::BOOLEAN) {
        throw std::invalid_argument("[ailIfFalse] predicate should be Boolean");
    }

    if (argumentType == Type::LABEL) {
        string label = argument;

        if (predicate == "#f") {
            int targetAddress = this->currentProcessPtr->labelAddressMap[label];
            this->currentProcessPtr->gotoAddress(targetAddress);
        } else {
            this->currentProcessPtr->step();
        }
    } else {
        throw std::invalid_argument("[ailIfFalse] argument should be Label");
    }

}

void Runtime::ailGoto() {
    string argument = this->currentProcessPtr->currentInstruction().argument;
    Type argumentType = typeOfStr(argument);
    if (argumentType == Type::LABEL) {
        string label = argument;
        int targetAddress = this->currentProcessPtr->labelAddressMap[label];
        this->currentProcessPtr->gotoAddress(targetAddress);
    } else {
        throw std::invalid_argument("[ailGoto] argument should be Label");
    }
}

vector<HandleOrStr> Runtime::popOperandsInversely(int num) {
    vector<HandleOrStr> buffer;
    for (int j = 0; j < num; ++j) {
        buffer.insert(buffer.begin(), this->currentProcessPtr->popOperand());
    }
    return buffer;
}

void Runtime::ailBegin() {
    this->currentProcessPtr->step();
}


#endif // !RUNTIME_HPP