#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <stdexcept>
#include <memory>
#include "Instruction.hpp"
#include "Utils.hpp"
#include "ModuleLoader.hpp"
#include "schemeObject.hpp"

using namespace std;

typedef int PID;
typedef string Handle;

enum ProcessState {
    READY, RUNNING, SLEEPING, SUSPENDED, STOPPED
};

const Handle TOP_NODE_HANDLE = "&TOP_NODE";

class Heap {
public:
    map<Handle, std::shared_ptr<SchemeObject>> dataMap;
    int handleCounter;

    bool hasHandle(Handle handle);

    shared_ptr<SchemeObject> get(Handle handle);

    void set(Handle handle, shared_ptr<SchemeObject> schemeObjectPtr);
};



class StackFrame {
public:
    StackFrame(const Closure &closure, int returnAddress) : closure(closure), returnAddress(returnAddress) {}

    Closure closure;
    int returnAddress;
};

class Process {
public:
    vector<string> opStack;
    vector<StackFrame> fStack;
    vector<Instruction> instructions;
    ProcessState state = READY;
    Heap heap;
    PID pid = 0;
    int PC = 0;
    std::shared_ptr<Closure> currentClosurePtr;

    Process(PID newPid, const ModuleLoader &moduleLoader);

    Instruction currentInstruction();

    inline void step() { this->PC++; };

    string popOperand();

    void pushStackFrame(const Closure &closure, int returnAddress);

    StackFrame popStackFrame();

    void pushOperand(const string &value);

    string dereference(const string &variableName);
};


//=================================================================
//                    PROCESS
//=================================================================

Instruction Process::currentInstruction() {
    return this->instructions[PC];
}

Process::Process(PID newPid, const ModuleLoader &moduleLoader) {
    this->pid = newPid;
    // from module loader loads instruction (string -> instruction)
    for (auto &i : moduleLoader.ILCode) {
        this->instructions.emplace_back(Instruction(i));
    }
    this->currentClosurePtr = std::shared_ptr<Closure>(new Closure(-1, nullptr));
    this->heap.set(TOP_NODE_HANDLE, this->currentClosurePtr);
};

void Process::pushOperand(const string &value) {
    this->opStack.push_back(value);
}

string Process::popOperand() {
    string popValue = this->opStack.back();
    this->opStack.pop_back();
    return popValue;
}

void Process::pushStackFrame(const Closure &closure, int returnAddress) {
    StackFrame sf(closure, returnAddress);
    this->fStack.push_back(sf);
}

StackFrame Process::popStackFrame() {
    StackFrame sf = this->fStack.back();
    return sf;
}

void Process::pushCurrentClosure(int returnAddress) {
    StackFrame sf(closure, returnAddress);
    this->fStack.push_back(sf);
}

string Process::dereference(const string &variableName) {

}


//=================================================================
//                           Heap
//=================================================================

bool Heap::hasHandle(Handle handle) {
    return this->dataMap.count(handle);
}

std::shared_ptr<SchemeObject> Heap::get(Handle handle) {
    if (this->hasHandle(std::move(handle))) {
        return this->dataMap[handle];
    } else {
        throw std::out_of_range("[ERROR] handle holds nothing -- Heap::get");
    }
}

void Heap::set(Handle handle, std::shared_ptr<SchemeObject> schemeObjectPtr) {
    this->dataMap[handle] = schemeObjectPtr;
}


#endif // !PROCESS_HPP