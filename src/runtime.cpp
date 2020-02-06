#include "runtime.hpp"
#include "process.hpp"

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <iostream>
using namespace std;


int main() {
    PID a = 12;
    cout << "hello";
    return 0;
};

void Runtime::setPID(int number){
    m_PID += number;
    return;
};

void Runtime::execute(Process process) {
    Instruction instruction = process.currentInstruction();
}