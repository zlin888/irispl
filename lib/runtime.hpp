#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "process.hpp"

#include <string>
#include <map>
#include <queue>

using namespace std;

class Runtime{
    public:
        map<PID, Process> processPool;
        queue<Process> processQueue;

        void start();
        void execute(Process process);
        int addProcess(Process process);
        Process createProcess();
        PID allocatePID();
};


//=================================================================
//                      PROCESS RELATED
//=================================================================
int Runtime::addProcess(Process process) {
    if(!processPool.count(process.pid)){
        //process not in pool 
        processPool.emplace(process.pid, process);
    }
    processQueue.push(process);
    return process.pid;
};

Process Runtime::createProcess() {
    Process process(this->allocatePID());
    return process;
}

PID Runtime::allocatePID(){
    return processPool.size();
}

void Runtime::execute(Process process) {
    Instruction instruction = process.currentInstruction();
    if(instruction.type == "COMMENT" || instruction.type == "LABEL") {
    }
    else {
    }
}

#endif // !RUNTIME_HPP