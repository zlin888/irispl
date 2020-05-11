#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "Process.hpp"
#include "ModuleLoader.hpp"
#include "IrisObject.hpp"

#include <string>
#include <map>
#include <queue>
#include <stdexcept>
#include <cmath>

using namespace std;

string RUNTIME_PREFIX = "_!!!runtime_prefix!!!_";
string RUNTIME_PREFIX_TITLE = "Runtime Error";
string PUSHEND = "_!!!pushend!!!_";

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
    vector<string> pushendStack;
    bool pushendMode = false;

    string ERROR_PREFIX = "------------ Runtime Error ------------\n";
    string ERROR_POSTFIX = "---------------------------------------";

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

    void ailCar();

    void ailCdr();

    void ailList();

    void ailIfTrue();

    void ailIfFalse();

    void ailGoto();

    vector<HandleOrStr> popOperands(int num);

    string toStr(HandleOrStr hos);

    void ailBegin();

    void ailIsPair();

    void ailPushend();

    vector<HandleOrStr> popOperandsToPushend();

    void checkWrongArgumentsNumberError(string functionName, int expectedNum, int actualNum);

    void addPushendStack(string pushendStr);

    bool matchPushendStack(string pushendStr);

    void ailPushlist();

    void ailPushlist(Handle listHandle);

    void ailCons();

    void ailExit();

    void ailType();

    string toType(HandleOrStr hos);

    string doubleToStr(double trouble);

    void execute(Instruction instruction);

    bool areHosesEqual(vector<HandleOrStr> hoses1, vector<HandleOrStr> hoses2);

    bool isEq(HandleOrStr operand1, HandleOrStr operand2);
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
            if (this->currentProcessPtr->state == ProcessState::RUNNING) {
                continue;
            }
            break;
        }

        // put the unfinished process to the back of the queue
        if (this->currentProcessPtr->state == ProcessState::RUNNING) {
            this->currentProcessPtr->state = ProcessState::READY;
            this->processQueue.push(this->currentProcessPtr);
        }
    }
}

void Runtime::execute(Instruction instruction) {
    if (instruction.type != InstructionType::COMMENT && instruction.type != InstructionType::LABEL) {
        string argument = instruction.argument;
        string mnemonic = instruction.mnemonic;

//        try {
        if (mnemonic == "store") { this->ailStore(); }
        else if (mnemonic == "load") { this->ailLoad(); }
        else if (mnemonic == "loadclosure") { this->aliLoadClosure(); }
        else if (mnemonic == "push") { this->ailPush(); }
        else if (mnemonic == "pushend") { this->ailPushend(); }
        else if (mnemonic == "pushlist") { this->ailPushlist(); }
        else if (mnemonic == "pop") { this->ailPop(); }
        else if (mnemonic == "set") { this->ailSet(); }
        else if (mnemonic == "type") { this->ailType(); }


        else if (mnemonic == "return") { this->ailReturn(); }
        else if (mnemonic == "iftrue") { this->ailIfTrue(); }
        else if (mnemonic == "iffalse") { this->ailIfFalse(); }
        else if (mnemonic == "goto") { this->ailGoto(); }

        else if (mnemonic == "car") { this->ailCar(); }
        else if (mnemonic == "cdr") { this->ailCdr(); }
        else if (mnemonic == "list") { this->ailList(); }
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
        else if (mnemonic == "pair?") { this->ailIsPair(); }


        else if (mnemonic == "fork") { this->ailFork(); }
        else if (mnemonic == "display") { this->ailDisplay(); }
        else if (mnemonic == "newline") { this->ailNewline(); }
        else if (mnemonic == "read") { this->ailRead(); }
        else if (mnemonic == "write") { this->ailWrite(); }
        else if (mnemonic == "nop") { this->ailNop(); }
        else if (mnemonic == "pause") { this->ailPause(); }
        else if (mnemonic == "halt") { this->ailHalt(); }
        else if (mnemonic == "begin") { this->ailBegin(); }
        else if (mnemonic == "exit") { this->ailExit(); }

        else if (mnemonic == "set-child!") { this->ailSetchild(); }
        else if (mnemonic == "concat") { this->ailConcat(); }
        else if (mnemonic == "duplicate") { this->ailDuplicate(); }

        else if (mnemonic == "call") { this->ailCall(instruction); }
        else if (mnemonic == "tailcall") { this->ailTailCall(instruction); }
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

void Runtime::execute() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    this->execute(instruction);
}


//=================================================================
//              Basic Instruction : Load and Store
//=================================================================

void Runtime::ailStore() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    if (instruction.argumentType != InstructionArgumentType::VARIABLE)
        throw std::invalid_argument("[ERROR] store argument is not a variable : aliStore");

    string variableName = instruction.argument;

    // handle the argument '.'
    // store arbitrary-args.scm.LAMBDA_12..
    // store arbitrary-args.scm.LAMBDA_12.args
    // we need to set the global variable PUSHEND_MODE
    // treat args as list, and store everything from stack to it until we reach PUSHEND
    if (variableName.ends_with('.')) {
        // make sure the next instruction looks like 'store agrs'
        // it this handle in compiler, no need to handle again
        Instruction nextInstruction = this->currentProcessPtr->nextInstruction();
        if (nextInstruction.mnemonic != "store") {
            utils::log("When using arbitrary arguments function, an variable to stored a list must be put after '.'.",
                       __FILE__, __FUNCTION__, __LINE__);
            throw std::runtime_error("");
        }
        this->pushendMode = true;
        this->currentProcessPtr->step();

        // do not store anything on '.'
        return;
    }

    if (this->pushendMode) {
        Handle handle = this->currentProcessPtr->heap.makeList(RUNTIME_PREFIX, TOP_NODE_HANDLE);
        shared_ptr<ListObject> listObjPtr = static_pointer_cast<ListObject>(this->currentProcessPtr->heap.get(handle));

        vector<HandleOrStr> hoses = this->popOperandsToPushend();
        for (auto hos : hoses) {
            listObjPtr->addChild(hos);
        }

        this->pushendMode = false;
        this->currentProcessPtr->currentClosurePtr->setBoundVariable(variableName, handle, false);
    } else {
        auto hoses = this->popOperands(1);
        string variableValue = hoses[0];
        this->currentProcessPtr->currentClosurePtr->setBoundVariable(variableName, variableValue, false);
    }

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
        this->currentProcessPtr->getClosurePtr(newClosureHandle)->setFreeVariable(boundVariable.first,
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

void Runtime::ailPushend() {
    Instruction instruction = this->currentProcessPtr->currentInstruction();
    this->currentProcessPtr->pushOperand(PUSHEND + "." + instruction.argument);
    this->currentProcessPtr->step();
}

void Runtime::ailPushlist() {

    auto hoses = this->popOperands(1);
    // TODO: raise a type error here
    auto listObjPtr = static_pointer_cast<ListObject>(this->currentProcessPtr->heap.get(hoses[0]));

    for (int i = listObjPtr->realListObjPtr->size() - 1; i >= listObjPtr->currentIndex; i--) {
        this->currentProcessPtr->pushOperand(listObjPtr->realListObjPtr->childrenHoses[i]);
    }

    this->currentProcessPtr->step();
}

void Runtime::ailPop() {
    this->popOperands(1);
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

    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("set", 1, hoses.size());
    string newValue = hoses[0];

    if (!this->currentProcessPtr->currentClosurePtr->hasBoundVariable(variable) &&
        !this->currentProcessPtr->currentClosurePtr->hasFreeVariable(variable)) {
        utils::log("variable " + variable + " not found", __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    if (this->currentProcessPtr->currentClosurePtr->hasBoundVariable(variable)) {
        this->currentProcessPtr->currentClosurePtr->setBoundVariable(variable, newValue, true);
    }

    if (this->currentProcessPtr->currentClosurePtr->hasFreeVariable(variable)) {
        this->currentProcessPtr->currentClosurePtr->setFreeVariable(variable, newValue, true);
    }

    // track up, change the parent closure
    auto currentClosurePtr = this->currentProcessPtr->currentClosurePtr->parentClosurePtr;
    auto topClosurePtr = this->currentProcessPtr->getClosurePtr(TOP_NODE_HANDLE);

    while (currentClosurePtr != topClosurePtr) {
        if (currentClosurePtr->hasBoundVariable(variable)) {
            currentClosurePtr->setBoundVariable(variable, newValue, true);
        }

        currentClosurePtr = currentClosurePtr->parentClosurePtr;
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
        shared_ptr<IrisObject> schemeObjPtr = this->currentProcessPtr->heap.get(handle);

        if (schemeObjPtr->irisObjectType == IrisObjectType::CLOSURE) {
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
    } else if (instruction.argumentType == InstructionArgumentType::KEYWORD) {
        string newArgument;
        if (primitiveInstructionMap.count(instruction.argument)) {
            newArgument = primitiveInstructionMap[instruction.argument];
        } else {
            newArgument = instruction.argument;
        }
        Instruction newInstruction(newArgument);
        this->execute(newInstruction);
//    } else if (typeOfStr(instruction.mnemonic) == InstructionArgumentType::KEYWORD) {
//        Instruction newInstruction(instruction.mnemonic);
//        this->ailCall(newInstruction);
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

void Runtime::ailExit() {
    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("exit", 1, hoses.size());

    this->output(this->toStr(hoses[0]), true);
    this->currentProcessPtr->state = ProcessState::STOPPED;
}


//=================================================================
//                      Calculation
//=================================================================

void Runtime::ailAdd() {
    auto hoses = this->popOperands(2);
    if (hoses.size() != 2) {
        string errorMessage = utils::createArgumentsNumberErrorMessage("+", 2, hoses.size());
        utils::raiseError(errorMessage, RUNTIME_PREFIX_TITLE);
    }
    string operand1 = hoses[0];
    string operand2 = hoses[1];

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        // TODO: NUMBER is not enough, we need better integer system here
        double result = stod(operand1) + stod(operand2);
        string resultStr;
        if (utils::double_is_int(result)) {
            resultStr = to_string((int) result);
        } else {
            resultStr = to_string(result);
        }
        this->currentProcessPtr->pushOperand(resultStr);
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();
}

string Runtime::doubleToStr(double trouble) {
    if (utils::double_is_int(trouble)) {
        return to_string((int) trouble);
    } else {
        return to_string(trouble);
    }

}


void Runtime::ailSub() {
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("sub", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

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
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("div", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

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
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("mul", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

    if (Instruction::getArgumentType(operand1) == InstructionArgumentType::NUMBER &&
        Instruction::getArgumentType(operand2) == InstructionArgumentType::NUMBER) {
        double result = stod(operand1) * stod(operand2);
        this->currentProcessPtr->pushOperand(this->doubleToStr(result));
    } else {
        utils::log("need two numbers, but gets " + operand1 + " and " + operand2, __FILE__, __FUNCTION__, __LINE__);
        throw std::invalid_argument("");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailMod() {
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("mod", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

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
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("pow", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

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
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("eqn", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

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
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("ge", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

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
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("le", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

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
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("gt", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

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
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("lt", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

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
    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("lt", 1, hoses.size());
    string operand = hoses[0];

    this->currentProcessPtr->pushOperand(operand == "#f" ? "#t" : "#f");
    this->currentProcessPtr->step();
}

void Runtime::ailAnd() {
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("and", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

    if (operand1 == "#f" || operand2 == "#f") {
        this->currentProcessPtr->pushOperand("#f");
    } else {
        this->currentProcessPtr->pushOperand("#t");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailOr() {
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("or", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

    if (operand1 == "#t" || operand2 == "#t") {
        this->currentProcessPtr->pushOperand("#t");
    } else {
        this->currentProcessPtr->pushOperand("#f");
    }

    this->currentProcessPtr->step();
}

void Runtime::ailIsEq() {
    auto hoses = this->popOperands(2);
    this->checkWrongArgumentsNumberError("eq", 2, hoses.size());
    string operand1 = hoses[0];
    string operand2 = hoses[1];

    if(this->isEq(operand1, operand2)) {
        this->currentProcessPtr->pushOperand("#t");
    } else {
        this->currentProcessPtr->pushOperand("#f");
    }

    this->currentProcessPtr->step();
}

bool Runtime::isEq(HandleOrStr operand1, HandleOrStr operand2) {
    Type operand1Type = typeOfStr(operand1);
    Type operand2Type = typeOfStr(operand2);

    if (operand1Type != operand2Type) {
        return false;
    }

    if (operand1Type == Type::HANDLE) {
        auto schemeObjPtr1 = this->currentProcessPtr->heap.get(operand1);
        auto schemeObjPtr2 = this->currentProcessPtr->heap.get(operand2);

        if (schemeObjPtr1->irisObjectType != schemeObjPtr2->irisObjectType) {
            return false;
        }

        if (schemeObjPtr1->irisObjectType == IrisObjectType::QUOTE) {
            auto hoses1 = IrisObject::getChildrenHosesOrBodies(schemeObjPtr1);
            auto hoses2 = IrisObject::getChildrenHosesOrBodies(schemeObjPtr2);

            if (this->areHosesEqual(hoses1, hoses2)) {
                return true;
            } else {
                return false;
            }

        } else if (schemeObjPtr1->irisObjectType == IrisObjectType::LIST) {
            auto l1ObjPtr = static_pointer_cast<ListObject>(schemeObjPtr1);
            auto l2ObjPtr = static_pointer_cast<ListObject>(schemeObjPtr2);

            auto hoses1 = l1ObjPtr->getChildrenHoses();
            auto hoses2 = l2ObjPtr->getChildrenHoses();

            if (this->areHosesEqual(hoses1, hoses2)) {
                return true;
            } else {
                return false;
            }
        }
    }

    if (operand1 == operand2) {
        return true;
    } else {
        return false;
    }
}

bool Runtime::areHosesEqual(vector<HandleOrStr> hoses1, vector<HandleOrStr> hoses2) {
    if (hoses1.size() != hoses2.size()) {
        return false;
    }

    for (int i = 0; i < hoses1.size(); ++i) {
        if (!this->isEq(hoses1[i], hoses2[i])) {
            return false;
        }
    }
    return true;
}

void Runtime::ailIsnull() {

}

void Runtime::ailIsatom() {

}

void Runtime::ailIsList() {
    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("isList", 1, hoses.size());
    string argument = hoses[0];

    if (typeOfStr(argument) == Type::HANDLE) {
        auto schemeObjPtr = this->currentProcessPtr->heap.get(argument);
        if (schemeObjPtr->irisObjectType != IrisObjectType::LIST) {
            this->currentProcessPtr->pushOperand("#f");
        } else {
            this->currentProcessPtr->pushOperand("#t");
        }
    } else {
        this->currentProcessPtr->pushOperand("#f");
    }
    this->currentProcessPtr->step();
}

void Runtime::ailIsPair() {
    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("isPair", 1, hoses.size());
    string argument = hoses[0];

    if (typeOfStr(argument) == Type::HANDLE) {
        auto schemeObjPtr = this->currentProcessPtr->heap.get(argument);
        if (schemeObjPtr->irisObjectType != IrisObjectType::LIST) {
            this->currentProcessPtr->pushOperand("#f");
        } else {
            auto listObjPtr = static_pointer_cast<ListObject>(schemeObjPtr);
            if (listObjPtr->size() <= 1) {
                this->currentProcessPtr->pushOperand("#f");
            } else {
                this->currentProcessPtr->pushOperand("#t");
            }
        }
    } else {
        this->currentProcessPtr->pushOperand("#f");
    }
    this->currentProcessPtr->step();

}

void Runtime::ailIsnumber() {
    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("isNumber", 1, hoses.size());
    string operand = hoses[0];

    this->currentProcessPtr->pushOperand(typeOfStr(operand) == Type::NUMBER ? "#t" : "#f");
    this->currentProcessPtr->step();
}


void Runtime::ailType() {
    auto hoses = this->popOperands(1);
    string hos = hoses[0];

    Handle quoteHandle = this->currentProcessPtr->heap.makeQuote(RUNTIME_PREFIX, TOP_NODE_HANDLE);
    auto quoteObjPtr = static_pointer_cast<QuoteObject>(this->currentProcessPtr->heap.get(quoteHandle));
    quoteObjPtr->addChild(toType(hos));

    this->currentProcessPtr->pushOperand(quoteHandle);

    this->currentProcessPtr->step();
}

string Runtime::toType(HandleOrStr hos) {
    Type hosType = typeOfStr(hos);
    if (hosType == Type::HANDLE) {
        shared_ptr<IrisObject> schemeObjectPtr = this->currentProcessPtr->heap.get(hos);
        return IrisObjectTypeStrMap[schemeObjectPtr->irisObjectType];
    } else {
        return TypeStrMap[hosType];
    }
}



//=================================================================
//                      Other Instructions
//=================================================================

void Runtime::ailDisplay() {
    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("display", 1, hoses.size());
    string argument = hoses[0];

    this->output(this->toStr(argument), true);
    this->currentProcessPtr->step();
}

string Runtime::toStr(HandleOrStr hos) {
    Type hosType = typeOfStr(hos);
    if (hosType == Type::HANDLE) {
        shared_ptr<IrisObject> schemeObjectPtr = this->currentProcessPtr->heap.get(hos);
        if (schemeObjectPtr->irisObjectType == IrisObjectType::STRING) {
            auto stringObjPtr = static_pointer_cast<StringObject>(schemeObjectPtr);
            return stringObjPtr->content;
        } else if (schemeObjectPtr->irisObjectType == IrisObjectType::QUOTE) {
            string buffer = "(";
            auto hoses = IrisObject::getChildrenHosesOrBodies(schemeObjectPtr);
            for (int i = 0; i < hoses.size(); ++i) {
                buffer += this->toStr(hoses[i]);
                if (i != hoses.size() - 1) {
                    buffer += " ";
                } else {
                    buffer += ")";
                }
            }
            if (hoses.size() == 0) {
                buffer += ")";
            }
            return buffer;
        } else if (schemeObjectPtr->irisObjectType == IrisObjectType::CLOSURE) {
            return "<lambda: " + hos + " >";
        } else if (schemeObjectPtr->irisObjectType == IrisObjectType::LIST) {
            auto listObjPtr = static_pointer_cast<ListObject>(schemeObjectPtr);
            int currentIndex = listObjPtr->currentIndex;
            string buffer = "(";
            auto hoses = IrisObject::getChildrenHosesOrBodies(listObjPtr->realListObjPtr);
            for (int i = currentIndex; i < hoses.size(); ++i) {
                buffer += this->toStr(hoses[i]);
                if (i != hoses.size() - 1) {
                    buffer += " ";
                } else {
                    buffer += ")";
                    return buffer;
                }
            }
            buffer += ")"; //always finished brackets
            return buffer;
        } else {
            return hos + " " + irisObjectTypeToStr(schemeObjectPtr->irisObjectType);
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
    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("car", 1, hoses.size());

    HandleOrStr hos = hoses[0];
    Type hosType = typeOfStr(hos);

    if (hosType == Type::HANDLE) {
        shared_ptr<IrisObject> schemeObjectPtr = this->currentProcessPtr->heap.get(hos);
        if (schemeObjectPtr->irisObjectType == IrisObjectType::LIST) {
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
    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("cdr", 1, hoses.size());

    HandleOrStr hos = hoses[0];
    Type hosType = typeOfStr(hos);

    if (hosType == Type::HANDLE) {
        shared_ptr<IrisObject> schemeObjectPtr = this->currentProcessPtr->heap.get(hos);
        if (schemeObjectPtr->irisObjectType == IrisObjectType::LIST) {
            shared_ptr<ListObject> listObjPtr = static_pointer_cast<ListObject>(schemeObjectPtr);


            Handle newListHandle = this->currentProcessPtr->heap.makeList(RUNTIME_PREFIX, TOP_NODE_HANDLE);
            auto newListObjPtr = static_pointer_cast<ListObject>(this->currentProcessPtr->heap.get(newListHandle));

            if (listObjPtr->isFake) {
                newListObjPtr->pointTo(listObjPtr->realListObjPtr, listObjPtr->currentIndex + 1);
            } else {
                newListObjPtr->pointTo(listObjPtr, 1);
            }

            this->currentProcessPtr->pushOperand(newListHandle);

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

void Runtime::ailList() {
    // create list, and push it
    // is actually push handle_to_list

    Handle handle = this->currentProcessPtr->heap.makeList(RUNTIME_PREFIX, TOP_NODE_HANDLE);
    shared_ptr<ListObject> listObjPtr = static_pointer_cast<ListObject>(this->currentProcessPtr->heap.get(handle));
//    string argumentNum = this->currentProcessPtr->popOperand();

    auto hoses = this->popOperandsToPushend();
    for (auto hos : hoses) {
        listObjPtr->addChild(hos);
    }

    this->currentProcessPtr->pushOperand(handle);
    this->currentProcessPtr->step();
}

void Runtime::ailCons() {
    auto hoses = this->popOperands(2);
    Handle handle = this->currentProcessPtr->heap.makeList(RUNTIME_PREFIX, TOP_NODE_HANDLE);
    shared_ptr<ListObject> consListObjPtr = static_pointer_cast<ListObject>(this->currentProcessPtr->heap.get(handle));

    for (auto hos :hoses) {
        Type hosType = typeOfStr(hos);
        if (hosType == Type::HANDLE) {
            auto schemeObjPtr = static_pointer_cast<ListObject>(this->currentProcessPtr->heap.get(hos));
            if (schemeObjPtr->irisObjectType == IrisObjectType::LIST) {
                auto listObjPtr = static_pointer_cast<ListObject>(this->currentProcessPtr->heap.get(hos));
                for (int i = listObjPtr->currentIndex; i < listObjPtr->realListObjPtr->childrenHoses.size(); i++) {
                    consListObjPtr->addChild(listObjPtr->childrenHoses[i]);
                }
            } else {
                consListObjPtr->addChild(hos);
            }
        } else {
            consListObjPtr->addChild(hos);
        }
    }

    this->currentProcessPtr->pushOperand(handle);
    this->currentProcessPtr->step();
}

void Runtime::ailIfTrue() {
    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("iftrue", 1, hoses.size());

    string predicate = hoses[0];
    Type predicateType = typeOfStr(predicate);

    string argument = this->currentProcessPtr->currentInstruction().argument;
    Type argumentType = typeOfStr(argument);

    if (predicateType != Type::BOOLEAN) {
        throw std::invalid_argument("[ailIfTrue] predicate should be Boolean");
    }

    if (argumentType == Type::LABEL) {
        string label = argument;
//
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
    auto hoses = this->popOperands(1);
    this->checkWrongArgumentsNumberError("iffalse", 1, hoses.size());

    string predicate = hoses[0];
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

vector<HandleOrStr> Runtime::popOperands(int num) {
    vector<HandleOrStr> buffer;
    for (int j = 0; j < num; ++j) {
        if (!this->currentProcessPtr->opStack.empty()) {
            HandleOrStr hos = this->currentProcessPtr->popOperand();
            if (hos.starts_with(PUSHEND)) {
                this->matchPushendStack(hos);
                j--;
            } else {
                buffer.insert(buffer.end(), hos);
            }
        } else {
            break;
        }
    }
    return buffer;
}

vector<HandleOrStr> Runtime::popOperandsToPushend() {
    vector<HandleOrStr> buffer;

    while (!this->currentProcessPtr->opStack.empty()) {
        auto hos = this->currentProcessPtr->popOperand();
        if (hos.starts_with(PUSHEND)) {
            if (this->matchPushendStack(hos)) {
                break;
            }
        } else {
            buffer.insert(buffer.end(), hos);
        }
    }
    return buffer;
}

bool Runtime::matchPushendStack(string pushendStr) {
    if (this->pushendStack.empty() || this->pushendStack.back() != pushendStr) {
        this->pushendStack.push_back(pushendStr);
        return false;
    } else {
        this->pushendStack.pop_back();
        return true;
    }
}

void Runtime::ailBegin() {
    this->currentProcessPtr->step();
}

void Runtime::checkWrongArgumentsNumberError(string functionName, int expectedNum, int actualNum) {
    if (expectedNum != actualNum) {
        string be = actualNum > 1 ? " are " : " is ";
        string add_s = expectedNum > 1 ? "s" : "";
        cout << this->ERROR_PREFIX;
        cout << "[" + functionName + "] expects " + to_string(expectedNum) + " argument" + add_s +
                ", " +
                to_string(actualNum) + be + "given" << endl;
        cout << this->ERROR_POSTFIX << endl;
    }
}


#endif // !RUNTIME_HPP