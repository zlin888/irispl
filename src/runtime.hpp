#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <string>
#include "process.hpp"

typedef int PID;

class Runtime{
    public:
        void setPID(int number);
        void execute(Process process);
        enum ProcessState {
            READY, RUNNING, SLEEPING, SUSPENDED, STOPPED
        };
    private:
        PID m_PID;
        PID m_parentPID;
};

#endif // !RUNTIME_HPP