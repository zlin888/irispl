#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <vector>

using namespace std;

enum ProcessState {
    READY, RUNNING, SLEEPING, SUSPENDED, STOPPED
};
class Instruction;{

};

class Process{
    public:
        Instruction currentInstruction();
    private:
        vector<string> opStack;
        vector<string> fStack;
};

Instruction Process::currentInstruction(){
    Instruction instruction;
    return instruction;
}

#endif // !PROCESS_HPP