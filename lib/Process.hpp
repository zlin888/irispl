#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <utility>
#include <vector>
#include "Instruction.hpp"
#include "Utils.hpp"
#include "ModuleLoader.hpp"

using namespace std;

typedef int PID;

enum ProcessState {
    READY, RUNNING, SLEEPING, SUSPENDED, STOPPED
};

class Process{
    public:
    Process(PID newPid, const ModuleLoader &moduleLoader);

    Instruction currentInstruction();
    void setProcessState(ProcessState ps);
    void inline step() {this->PC++;};

    vector<string> opStack;
    vector<string> fStack;
    vector<Instruction> instructions;
    ProcessState state = READY;
    PID pid;
    int PC = 0;
};

Instruction Process::currentInstruction(){
    return this->instructions[PC];
}

Process::Process(PID newPid, const ModuleLoader& moduleLoader) {
    this->pid = newPid;
    // from module loader loads instruction (string -> instruction)
    for ( auto &i : moduleLoader.ILCode ) {
        this->instructions.emplace_back(Instruction(i));
    }
};


void Process::setProcessState(ProcessState ps) {
    this->state = ps;
}

#endif // !PROCESS_HPP