#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "Process.hpp"
#include "ModuleLoader.hpp"

#include <string>
#include <map>
#include <queue>

using namespace std;

class Runtime{
    public:
        map<PID, Process> processPool;
        queue<Process> processQueue;

    void schedule();
    void execute(Process &process);
    int addProcess(Process process);
    Process createProcess(ModuleLoader moduleLoader);
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

// process factory, allocate PID and help to pass module loader to the constructor of the process
Process Runtime::createProcess(ModuleLoader moduleLoader) {
    Process process(this->allocatePID(), moduleLoader);
    return process;
}

PID Runtime::allocatePID(){
    return processPool.size();
}

void Runtime::execute(Process& process) {

    Instruction instruction = process.currentInstruction();
    if (instruction.type == COMMENT || instruction.type == LABEL) {
        process.step();
    } else {



    }

    if (process.PC >= process.instructions.size()){
        process.state = STOPPED;
    }
}


//=================================================================
//                      Scheduler
//=================================================================
void Runtime::schedule() {
    while(!this->processQueue.empty()) {
        // Pop from process queue
        Process currentProcess = this->processQueue.front();
        this->processQueue.pop();

        // Run the current process
        // Round Robin Scheduling
        // Set time slice as 20 (execute 20 instructions each time), then switch to next process
        currentProcess.state = RUNNING;
        for (int timeSlice = 0; timeSlice < 20; ++timeSlice) {
            this->execute(currentProcess);
            switch(currentProcess.state) {
                case RUNNING:
                    continue;
                default:
                    break;
            }
        }

        // put the unfinished process to the back of the queue
        if(currentProcess.state == RUNNING){
            currentProcess.state = READY;
            this->processQueue.push(currentProcess);
        }

    }
}

#endif // !RUNTIME_HPP