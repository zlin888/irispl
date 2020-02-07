#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <vector>

using namespace std;

typedef int PID;

enum ProcessState {
    READY, RUNNING, SLEEPING, SUSPENDED, STOPPED
};

//=================================================================
//                     Instruction Definition 
//=================================================================
class Instruction{
    public:
        string type;
        string argType;
        string instruction;
        string mnemonic;
        string argument;
        Instruction(string instString);
};


Instruction::Instruction(string instString){
    //process the instString and construct Instruction object
    string hi = instString; 
};

//=================================================================
//                     Process Definition 
//=================================================================
class Process{
    public:
        vector<string> opStack;
        vector<string> fStack;
        PID pid;

        Process(PID newPid){
            this->pid = newPid;
        }
        Instruction currentInstruction();
};

Instruction Process::currentInstruction(){
    Instruction instruction("hi");
    return instruction;
};

#endif // !PROCESS_HPP